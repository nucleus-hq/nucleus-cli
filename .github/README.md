# Nucleus CLI

A cli built for [nucleus-shell](github.com/xZepyx/nucleus-shell).

## Features

* Run, reload, and debug Nucleus Shell
* Install or uninstall Nucleus Shell and dependencies
* Update to latest, edge, or specific Git release
* Switch themes
* Call or show IPC commands

## Installation

### From Aur
```
yay -S nucleus-shell
```

### From Source

```
git clone https://github.com/xZepyx/nucleus-cli.git
cd nucleus-cli
cmake -B build -S .
cmake --build build -- -j$(nproc)
sudo cmake --install build
```

> [!NOTE]
> Any other packages on the aur are not official and not supported.
> The official package name is `nucleus-shell` on aur.

### Dependencies

Ensure the following tools are installed:

* `cmake`
* `make`
* `g++` (C++20)
* `curl`
* `unzip`
* `nlohmann-json`

## Usage

```
nucleus <command> [options]
```

### Commands

| Command     | Description                                                                 |
| ----------- | --------------------------------------------------------------------------- |
| `run`       | Launch Nucleus Shell. Use `--reload` to restart, `--debug` for debug mode.  |
| `kill/stop` | Kill all running Nucleus Shell instances.                                   |
| `install`   | Install Nucleus Shell and required dependencies.                            |
| `uninstall` | Uninstall Nucleus Shell and optionally dependencies.                        |
| `update`    | Update Nucleus Shell. Prompts for version: Latest, Edge, or Git.            |
| `theme`     | Switch shell theme. Usage: `nucleus theme switch <themeName>`               |
| `ipc`       | Interact with IPC. `show` to display, `call <target> <function>` to invoke. |

### Examples

```
nucleus run                  # Start shell normally
nucleus run --reload         # Reload shell
nucleus install              # Install shell and dependencies
nucleus update               # Update shell interactively
nucleus theme switch hypr    # Change theme to 'hypr'
nucleus ipc show             # Show IPC information
```

## License
Mit License
