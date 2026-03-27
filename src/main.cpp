#include <cstdlib>
#include <install/install.hpp>
#include <iostream>
#include <ipc/ipc.hpp>
#include <prompts/prompts.hpp>
#include <string>
#include <theme/theme.hpp>
#include <unistd.h>
#include <update/update.hpp>
#include <vector>

enum class RunMode { Normal, Debug, Reload };

RunMode parseRunMode(const std::string &extraArg) {
  if (extraArg == "--reload")
    return RunMode::Reload;
  if (extraArg == "--debug")
    return RunMode::Debug;
  return RunMode::Normal;
}

std::string resolveHelper(int choice) {
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
  std::cout << "  run [--reload|--debug]\n";
  std::cout << "  kill | stop\n";
  std::cout << "  install\n";
  std::cout << "  uninstall\n";
  std::cout << "  ipc show\n";
  std::cout << "  ipc call <target> <func>\n";
  std::cout << "  update\n";
  std::cout << "  theme switch <themeName>\n\n";
}

// CORE packages (system / base stack — install but NEVER auto-remove)
const std::vector<std::string> core_packages = {
    // Display server / compositor base
    "hyprland",
    "xdg-desktop-portal-hyprland",
    "xorg-xrandr",

    // Qt / UI stack
    "qt6ct","qt5ct",
    "qt5-wayland","qt6-wayland",
    "qt5-graphicaleffects","qt6-5compat","qt6-svg",

    // System services
    "networkmanager",      // :contentReference[oaicite:0]{index=0}
    "wireplumber",
    "bluez-utils",

    // Core desktop integration
    "network-manager-applet",
    "wl-clipboard",

    // Essential tools
    "jq","zenity","flatpak"
};


// PROJECT dependencies (safe to install/remove with your project)
const std::vector<std::string> dependencies = {
    // Your stack
    "nucleus-shell",
    "quickshell-git",
    "matugen-bin",

    // Hypr ecosystem extras
    "hyprpaper","hyprlock","hyprpicker","hyprsunset",

    // Wayland utilities
    "wf-recorder","grim","slurp","wl-color-picker",

    // Theming
    "kvantum","kvantum-qt5","papirus-icon-theme-git",

    // Terminal / shell
    "kitty","fish","starship",

    // Apps
    "firefox","nautilus",

    // Media / system utils
    "imagemagick","ffmpeg",
    "playerctl","brightnessctl","ddcutil","fastfetch",

    // Fonts
    "nerd-fonts","ttf-jetbrains-mono",
    "ttf-fira-code","ttf-firacode-nerd",
    "ttf-material-symbols-variable-git",
    "ttf-font-awesome","ttf-fira-sans"
};

int main(int argc, char *argv[]) {
  int i = 1;

  if (argc < 2) {
    printUsage();
    return 0;
  }

  while (i < argc) {
    std::string arg = argv[i];

    if (arg == "run") {
      std::string extraArg = (i + 1 < argc) ? argv[i + 1] : "";
      RunMode mode = parseRunMode(extraArg);

      switch (mode) {
      case RunMode::Reload:
        prompt::Stage("Reloading");
        std::system("pkill quickshell 2>/dev/null");
        std::system("sleep 1; quickshell --no-duplicate --daemonize -c nucleus-shell");
        i++;
        break;

      case RunMode::Debug:
        prompt::Stage("Running in debug mode");
        std::system("quickshell --no-duplicate -c nucleus-shell");
        i++;
        break;

      case RunMode::Normal:
        prompt::Stage("Starting shell");
        std::system("quickshell --no-duplicate --daemonize -c nucleus-shell");
        break;
      }

      i++;
    }

    else if (arg == "kill" || arg == "stop") {
      prompt::Stage("Stopping Nucleus Shell");
      std::system("pkill quickshell 2>/dev/null");
      i++;
    }

    else if (arg == "install") {
      prompt::Stage("Installing Nucleus Shell");

      int choice = installer::getAurHelper();
      std::string helper = resolveHelper(choice);

      if (helper.empty()) {
        prompt::Fail("Invalid AUR helper");
        return 1;
      }

      for (const auto &pkg : dependencies) {
        installer::installPackage(helper, pkg);
      }

      installer::installShell();
      prompt::Success("Installation complete");

      i++;
    }

    else if (arg == "uninstall") {
      prompt::Stage("Uninstalling Nucleus Shell");

      std::string confirmation = prompt::Ask("Uninstall Nucleus Shell? [y/N]:");
      if (confirmation == "y" || confirmation == "Y") {

        std::system("pkill quickshell 2>/dev/null");

        std::system("rm -rf $HOME/.config/nucleus-shell");
        std::system("rm -rf $HOME/.config/quickshell/nucleus-shell");

        std::string input = prompt::Ask("Remove system dependencies? (not recommended) [y/N]:");
        if (input == "y" || input == "Y") {

          for (const auto &pkg : core_packages) {
            prompt::Stage("Removing Package:", pkg);
            std::string cmd = "sudo pacman -Rns --noconfirm " + pkg;

            if (std::system(cmd.c_str()) != 0) {
              prompt::Fail("Failed removing " + pkg);
            }
          }

          std::system("curl -fsS https://api.counterapi.dev/v1/xzepyx/nucleus-shell/down >/dev/null 2>&1");

          prompt::Success("Uninstalled shell and core dependencies");
        } else {
          prompt::Success("Uninstalled shell only");
        }

      } else {
        prompt::Success("Uninstallation cancelled");
      }

      i++;
    }

    else if (arg == "ipc") {
      if (i + 1 >= argc) {
        prompt::Fail("Missing IPC action");
        return 1;
      }

      std::string action = argv[i + 1];

      if (action == "show") {
        ipc::show();
        i += 2;
      } else if (action == "call") {
        if (i + 3 >= argc) {
          prompt::Fail("Usage: ipc call <target> <func> [args]");
          return 1;
        }

        std::string args = (i + 4 < argc) ? argv[i + 4] : "";
        ipc::call(argv[i + 2], argv[i + 3], args);
        i += 5;
      } else {
        prompt::Fail("Invalid IPC command");
        return 1;
      }
    }

    else if (arg == "update") {
      prompt::Stage("Updating Nucleus Shell");

      std::cout << "1. Latest\n2. Edge\n3. Tag\n4. Git\n5. Git Branch\n";

      int choice = 0;
      while (true) {
        std::cout << "[?] Choice: ";
        std::cin >> choice;
        if (choice >= 1 && choice <= 5) break;
        std::cout << "Invalid choice\n";
      }

      update::UpdateMode mode = update::choiceToMode(choice);
      std::string tag;

      if (mode == update::UpdateMode::Tag) {
        std::cout << "Enter tag: ";
        std::cin >> tag;
      }

      std::string branchName;

      if (mode == update::UpdateMode::GitBranch) {
        std::cout << "Enter branch name: ";
        std::cin >> branchName;
      }

      update::perform(mode, tag, branchName);
      i++;
    }

    else if (arg == "theme") {
      if (i + 2 >= argc) {
        prompt::Fail("Usage: nucleus theme switch <themeName>");
        return 1;
      }

      std::string action = argv[i + 1];
      std::string themeName = argv[i + 2];

      if (action == "switch" || action == "change" || action == "set") {
        theme::change(themeName);
      } else {
        prompt::Fail("Invalid theme command");
      }

      i += 3;
    }

    else if (arg == "-h" || arg == "--help") {
      printUsage();
      i++;
    }

    else {
      prompt::Fail("Unknown command: " + arg);
      printUsage();
      return 1;
    }
  }

  return 0;
}
