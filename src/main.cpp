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

// Full dependency list (install only)
const std::vector<std::string> dependencies = {
    "hyprland","hyprpaper","hyprlock","hyprpicker",
    "wf-recorder","wl-clipboard","grim","slurp",
    "qt6ct","qt5ct","kvantum","kvantum-qt5",
    "kitty","fish","starship",
    "firefox","nautilus","network-manager-applet",
    "wl-color-picker","imagemagick","qt6-svg",
    "networkmanager","wireplumber","bluez-utils",
    "fastfetch","playerctl","brightnessctl",
    "papirus-icon-theme-git","hyprsunset",
    "nerd-fonts","ttf-jetbrains-mono",
    "ttf-fira-code","ttf-firacode-nerd",
    "ttf-material-symbols-variable-git",
    "ttf-font-awesome","ttf-fira-sans",
    "quickshell-git","matugen-bin","ffmpeg",
    "qt5-wayland","qt6-wayland","qt5-graphicaleffects","qt6-5compat",
    "xdg-desktop-portal-hyprland","xorg-xrandr",
    "zenity","jq","ddcutil","flatpak","nucleus-shell"
};

// SAFE uninstall list (only your stuff)
const std::vector<std::string> core_packages = {
    "nucleus-shell",
    "quickshell-git",
    "matugen-bin"
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

        std::string input = prompt::Ask("Remove core dependencies? [y/N]:");
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

      std::cout << "1. Latest\n2. Edge\n3. Tag\n4. Git\n";

      int choice = 0;
      while (true) {
        std::cout << "[?] Choice: ";
        std::cin >> choice;
        if (choice >= 1 && choice <= 4) break;
        std::cout << "Invalid choice\n";
      }

      update::UpdateMode mode = update::choiceToMode(choice);
      std::string tag;

      if (mode == update::UpdateMode::Tag) {
        std::cout << "Enter tag: ";
        std::cin >> tag;
      }

      update::perform(mode, tag);
      i++;
    }

    else if (arg == "theme") {
      if (i + 2 >= argc) {
        prompt::Fail("Usage: nucleus theme switch <themeName>");
        return 1;
      }

      std::string action = argv[i + 1];
      std::string themeName = argv[i + 2];

      if (action == "switch") {
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
