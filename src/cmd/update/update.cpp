#include "update.hpp"

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <filesystem>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <prompts/prompts.hpp>
#include <funcs/spinner.hpp>

namespace fs = std::filesystem;

namespace update {

inline const std::string CONFIG =
    std::string(getenv("HOME")) + "/.config/nucleus-shell/config/configuration.json";

inline const std::string QS_DIR =
    std::string(getenv("HOME")) + "/.config/quickshell/nucleus-shell";

inline const std::string REPO = "nucleus-hq/nucleus-shell";

inline const std::string API =
    "https://api.github.com/repos/" + REPO + "/releases";

inline size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    std::string* str = static_cast<std::string*>(userdata);
    str->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string fetchLatestTag(bool stableOnly)
{
    std::string response;

    CURL* curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("curl init failed");

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: nucleus-shell");

    curl_easy_setopt(curl, CURLOPT_URL, API.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK)
        throw std::runtime_error("curl request failed");

    auto releases = nlohmann::json::parse(response);

    nlohmann::json latest;

    for (auto& r : releases)
    {
        if (r.value("draft", true))
            continue;

        if (stableOnly && r.value("prerelease", false))
            continue;

        if (latest.is_null() ||
            r.value("published_at","") > latest.value("published_at",""))
        {
            latest = r;
        }
    }

    if (latest.is_null())
        throw std::runtime_error("Release resolution failed");

    std::string tag = latest.value("tag_name", "");

    if (!tag.empty() && tag[0] == 'v')
        tag.erase(0,1);

    return tag;
}

UpdateMode choiceToMode(int choice)
{
    switch (choice)
    {
        case 1: return UpdateMode::Stable;
        case 2: return UpdateMode::Edge;
        case 3: return UpdateMode::Tag;
        case 4: return UpdateMode::Git;
        case 5: return UpdateMode::GitBranch;
        default: throw std::runtime_error("Invalid update choice");
    }
}

void perform(UpdateMode mode, const std::string& inputTag, const std::string& branch)
{
    if (!fs::exists(CONFIG))
        throw std::runtime_error("configuration.json not found");

    std::ifstream cfg(CONFIG);
    nlohmann::json cfgJson;
    cfg >> cfgJson;

    std::string current =
        cfgJson.value("shell", nlohmann::json::object())
               .value("version","");

    if (current.empty())
        throw std::runtime_error("Current version not set");

    fs::path tmp = fs::temp_directory_path() / "nucleus-update";

    fs::remove_all(tmp);
    fs::create_directories(tmp);

    std::string latest;
    std::string latest_tag;

    if (mode == UpdateMode::Git)
    {
        latest = "RC";

        fs::path repo_dir = tmp / "repo";
        fs::path src_dir = repo_dir / "quickshell" / "nucleus-shell";

        std::string cloneCmd =
            "git clone --depth=1 https://github.com/" + REPO +
            ".git " + repo_dir.string();

        console::spinner("Cloning repository", cloneCmd);

        if (!fs::exists(src_dir))
            throw std::runtime_error("nucleus-shell directory missing in repo");

        std::string installCmd =
            "rm -rf '" + QS_DIR + "' && "
            "mkdir -p '" + QS_DIR + "' && "
            "cp -r '" + src_dir.string() + "'/* '" + QS_DIR + "'";

        console::spinner("Installing files", installCmd);

        cfgJson["shell"]["version"] = latest;

        std::ofstream out(CONFIG);
        out << cfgJson.dump(4);

        console::spinner("Reloading shell",
            "killall quickshell &>/dev/null || true; "
            "nohup quickshell -c nucleus-shell &>/dev/null & disown");

        std::cout << "[✓] Updated to nucleus-shell git-rc\n";

        return;
    }

    if (mode == UpdateMode::GitBranch)
    {
        latest = "RC";

        fs::path repo_dir = tmp / "repo";
        fs::path src_dir = repo_dir / "quickshell" / "nucleus-shell";

        if (branch.empty()) {
            throw std::runtime_error("No tag provided");
        }

        std::string cloneCmd =
            "git clone --depth=1 https://github.com/" + REPO +
            ".git " + "-b " + branch + " " + repo_dir.string();

        console::spinner("Cloning repository", cloneCmd);

        if (!fs::exists(src_dir))
            throw std::runtime_error("nucleus-shell directory missing in repo");

        std::string installCmd =
            "rm -rf '" + QS_DIR + "' && "
            "mkdir -p '" + QS_DIR + "' && "
            "cp -r '" + src_dir.string() + "'/* '" + QS_DIR + "'";

        console::spinner("Installing files", installCmd);

        cfgJson["shell"]["version"] = latest;

        std::ofstream out(CONFIG);
        out << cfgJson.dump(4);

        console::spinner("Reloading shell",
            "killall quickshell &>/dev/null || true; "
            "nohup quickshell -c nucleus-shell &>/dev/null & disown");

        std::cout << "[✓] Updated to nucleus-shell git-rc\n";

        return;
    }

    if (mode == UpdateMode::Tag)
    {
        if (inputTag.empty())
            throw std::runtime_error("No tag provided");

        latest = (inputTag[0]=='v') ? inputTag.substr(1) : inputTag;
        latest_tag = "v" + latest;
    }
    else
    {
        bool stable = (mode == UpdateMode::Stable);

        latest = fetchLatestTag(stable);
        latest_tag = "v" + latest;
    }

    if (latest == current)
    {
        std::cout << "[*] Already up to date (" << current << ")\n";
        return;
    }

    std::string zip = (tmp / "source.zip").string();

    fs::path root_dir = tmp / ("nucleus-shell-" + latest);
    fs::path src_dir = root_dir / "quickshell" / "nucleus-shell";

    std::string downloadCmd =
        "curl -fsSL https://github.com/" + REPO +
        "/archive/refs/tags/" + latest_tag +
        ".zip -o " + zip;

    console::spinner("Downloading nucleus-shell " + latest, downloadCmd);

    std::string unzipCmd =
        "unzip -q " + zip + " -d " + tmp.string();

    console::spinner("Extracting archive", unzipCmd);

    if (!fs::exists(src_dir))
        throw std::runtime_error("nucleus-shell directory missing in archive");

    std::string installCmd =
        "rm -rf '" + QS_DIR + "' && "
        "mkdir -p '" + QS_DIR + "' && "
        "cp -r '" + src_dir.string() + "'/* '" + QS_DIR + "'";

    console::spinner("Installing files", installCmd);

    cfgJson["shell"]["version"] = latest;

    std::ofstream out(CONFIG);
    out << cfgJson.dump(4);

    console::spinner("Reloading shell",
        "killall quickshell &>/dev/null || true; "
        "nohup quickshell -c nucleus-shell &>/dev/null & disown");

    std::cout << "[✓] Updated nucleus-shell: "
              << current << " -> " << latest << "\n";
}

}
