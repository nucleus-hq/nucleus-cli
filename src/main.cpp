#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <prompts/prompts.hpp>
#include <install/install.hpp>
#include <ipc/ipc.hpp>
#include <theme/theme.hpp>
#include <update/update.hpp>

// Notice: The header file stuff is probably a abomination as I learned how to use them while making this.

enum class RunMode { Normal, Debug, Reload };

// parse run flag
RunMode parseRunMode(const std::string &extraArg) {
    if (extraArg == "--reload")
        return RunMode::Reload;
    if (extraArg == "--debug")
        return RunMode::Debug;
    return RunMode::Normal;
}

std::string resolveHelper(int choice) { // Resolve a helper from installer::getAurHelper
    switch (choice) {
        case 1: return "yay";
        case 2: return "paru";
        case 3: return "trizen";
        case 4: return "pikaur";
        default: return "";
    }
}

void printUsage() {
    std::cout << "Usage: nucleus <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  run [--reload|--debug]      Start the shell (reload or debug optional)\n";
    std::cout << "  kill | stop                 Kill the running shell\n";
    std::cout << "  install                     Install nucleus-shell and dependencies\n";
    std::cout << "  uninstall                   Uninstall nucleus-shell (optionally remove dependencies)\n";
    std::cout << "  ipc show                    Show available IPC targets\n";
    std::cout << "  ipc call <target> <func>    Call IPC functions\n";
    std::cout << "  update                      Update nucleus-shell (choose Latest/Edge/Git)\n";
    std::cout << "  theme switch <themeName>    Switch to a different theme\n";
    std::cout << "\nExamples:\n";
    std::cout << "  nucleus run --reload\n";
    std::cout << "  nucleus update\n";
    std::cout << "  nucleus theme switch dark\n";
    std::cout << std::endl;
}

const std::vector<std::string> dependencies = {
    "hyprland", "hyprpaper", "hyprlock", "hyprpicker",
    "wf-recorder", "wl-clipboard", "grim", "slurp",
    "qt6ct", "qt5ct", "kvantum", "kvantum-qt5",
    "kitty", "fish", "starship", "firefox",
    "nautilus", "network-manager-applet", "wl-color-picker",
    "imagemagick", "qt6-svg", "networkmanager", "wireplumber",
    "bluez-utils", "fastfetch", "playerctl", "brightnessctl",
    "papirus-icon-theme-git", "hyprsunset", "nerd-fonts",
    "ttf-jetbrains-mono", "ttf-fira-code", "ttf-firacode-nerd",
    "ttf-material-symbols-variable-git", "ttf-font-awesome",
    "ttf-fira-sans", "quickshell-git", "matugen-bin", "ffmpeg",
    "qt5-wayland", "qt6-wayland", "qt5-graphicaleffects",
    "qt6-5compat", "xdg-desktop-portal-hyprland", "zenity",
    "jq", "ddcutil", "flatpak"
};

int main(int argc, char *argv[]) {
    int i = 1;

    if (argc < 2) {
        printUsage(); // show help if no arguments
        return 0;
    }

    while (i < argc) {
        std::string arg = argv[i];

        if (arg == "run") {

            // determine if there is an extra arg present
            std::string extraArg = (i + 1 < argc) ? argv[i + 1] : "";

            RunMode mode = parseRunMode(extraArg);

            switch (mode) {
                case RunMode::Reload:
                    prompt::Stage("Reloading");
                    std::system("pkill -f quickshell; pkill -f qs"); // kill both qs and quickshell instance
                    std::system("sleep 1; quickshell --no-duplicate --daemonize -c nucleus-shell"); // wait before loading
                    i++; // consume flag
                    break;

                case RunMode::Debug:
                    prompt::Stage("Running in debug mode");
                    std::system("quickshell --no-duplicate -c nucleus-shell");
                    i++; // consume flag
                    break;

                case RunMode::Normal:
                    prompt::Stage("Initializing");
                    std::system("quickshell --no-duplicate --daemonize -c nucleus-shell");
                    break;

                default:
                    prompt::Fail("Failed to run shell");
                    std::cout << "Available Extra Commands:\n";
                    std::cout << "--reload, --debug\n";
                    break;
            }

            i++;
        } 
        else if (arg == "kill" || arg == "stop") {
            prompt::Stage("Killing Nucleus Shell");
            std::system("pkill -f quickshell; pkill -f qs");
            i++;
        } 
        else if (arg == "install") {
            prompt::Stage("Installating Nucleus Shell");
            int choice = installer::getAurHelper();
            std::string helper = resolveHelper(choice);

            if (!helper.empty()) {
                for (const auto& pkg : dependencies) {
                    installer::installPackage(helper, pkg); // Install all pkgs one by one
                }
                installer::installShell();
            } else {
                prompt::Fail("Invalid AUR helper\n");
            }

            i++;
        } 
        else if (arg == "uninstall") {
            prompt::Stage("Uninstallating Nucleus Shell");
            std::string confirmation = prompt::Ask("Uninstall Nucleus Shell? [y/N]:");
            if (confirmation == "" || confirmation == "y" || confirmation == "Y") {
                std::system("rm -rf ~/.config/nucleus-shell; rm -rf ~/.config/quickshell/nucleus-shell");
                std::string input = prompt::Ask("Uninstall all dependencies? [y/N]:");
                if (input == "y" || input == "Y" || input == "") {
                    for (const auto& pkg : dependencies) {
                        prompt::Stage("Removing Package:", pkg);

                        std::string installCmd = "pacman -Rns --noconfirm " + pkg;
                        std::system(installCmd.c_str());
                        prompt::Success("Uninstalled the Shell");
                    }
                }
                else {
                    prompt::Success("Uninstalled the shell wihout removing dependencies");
                }
            }
            else {
                prompt::Success("Stopped Uninstallation");
            }
            i++;
        } 
        else if (arg == "ipc") {
            std::string action = (i + 1 < argc) ? argv[i + 1] : ""; // get the action (either call or show)
            if (action == "show") {
                ipc::show();
		i+=2;
            }
            else if (action == "call") {
                std::string args = (i + 4 < argc) ? argv[i + 4] : "";
                ipc::call(argv[i+2], argv[i+3], args);
		i+=5;
            }
            else {
                prompt::Fail("No Target/Function Passed");
		i+=2;
            }
        }
        else if (arg == "update") {
            prompt::Stage("Updating Nucleus Shell");

            // Prompt user for update mode
            std::cout << "Select the version to install:\n";
            std::cout << "1. Latest\n";
            std::cout << "2. Edge\n";
            std::cout << "3. Git\n";

            int choice = 0;
            while (true) {
                std::cout << "[?] Choice: ";
                std::cin >> choice;

                if (choice >= 1 && choice <= 3) break;
                std::cout << "Invalid choice, try again.\n";
            }

            update::UpdateMode mode = update::choiceToMode(choice);
            std::string gitTag;

            // If Git mode, ask for tag
            if (mode == update::UpdateMode::Git) {
                std::cout << "[?] Enter Git tag: ";
                std::cin >> gitTag;
            }

            // Perform the update
            update::perform(mode, gitTag);

            i++;
        }
        else if (arg == "theme") {
            std::string action = (i + 1 < argc) ? argv[i + 1] : "";
            if (action == "switch") {
                std::string themeName = (i + 2 < argc) ? argv[i + 2] : "";
                theme::change(themeName);
            }
            else {
                prompt::Fail("Usage: nucleus theme switch <themeName>");
            }
            i++;
        }
	else if (arg == "-h" || arg == "--help") {
	    printUsage();
	    i++;
	}
	else {
           i+=argc;
	} 
    }

    return 0;
}
