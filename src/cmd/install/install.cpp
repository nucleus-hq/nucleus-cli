#include <cstdlib>
#include <install/install.hpp>
#include <iostream>
#include <prompts/prompts.hpp>
#include <string>

namespace installer {

int getAurHelper() {
  std::cout << "[?] Select AUR helper:\n"; // use cout instead of prompt to prevent flushing
  std::cout << "1. yay\n";
  std::cout << "2. paru\n";
  std::cout << "3. trizen\n";
  std::cout << "4. pikaur\n";
  std::cout << "[?] Choice: ";

  std::string input;
  std::getline(std::cin, input);

  if (input == "1")
    return 1;
  if (input == "2")
    return 2;
  if (input == "3")
    return 3;
  if (input == "4")
    return 4;

  prompt::Fail("Invalid selection\n");
  return -1;
}

void installPackage(const std::string &helper, const std::string &pkg) {
    // Check if package is already installed
    std::string checkCmd = "pacman -Qi " + pkg + " > /dev/null 2>&1";

    if (std::system(checkCmd.c_str()) == 0) {
        prompt::Stage("Skipping (already installed):", pkg);
        return;
    }

    prompt::Stage("Installing Package:", pkg);

    std::string installCmd = helper + " -S --noconfirm " + pkg;
    std::system(installCmd.c_str());
}


void installShell() {
  const std::string repoUrl = "https://github.com/nucleus-hq/nucleus-shell.git";
  const std::string cloneDir = "/tmp/nucleus-shell";

  // Clone repository
  std::string cloneCmd = "git clone " + repoUrl + " " + cloneDir;
  if (std::system(cloneCmd.c_str()) != 0) {
    prompt::Fail("Failed to clone repository\n");
    return;
  }

  // Create config directory
  std::string mkdirCmd = "mkdir -p ~/.config/quickshell";
  std::system(mkdirCmd.c_str());

  // Copy folder
  std::string copyCmd =
      "cp -r " + cloneDir + "/quickshell/nucleus-shell ~/.config/quickshell/";

  if (std::system(copyCmd.c_str()) != 0) {
    prompt::Fail("Failed to copy shell configuration\n");
    return;
  }

  std::system("curl https://api.counterapi.dev/v1//nucleus-shell/up"); // increments install counter on the web. (does not take personal info)
  prompt::Success("Shell installed successfully.\n");
}

} // namespace installer
