# VS Code Dev Container

This is a docker-based interactive development environment using VS Code and Docker Dev Containers removing the need to install any tools locally\*

## Requirements

- VS Code
  - [Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension
- Docker
- OpenOCD - For debugging

## Using

### Code editing, and building.

1. Clone InfiniTime and update submodules
2. Launch VS Code
3. Open InfiniTime directory,
4. Allow VS Code to open folder with devcontainer.

After this the environment will be built if you do not currently have a container setup, it will install all the necessary tools and extra VSCode extensions.

In order to build InfiniTime we need to run the initial submodule init and CMake commands.

#### Manually

You can use the VS Code terminal to run the CMake commands as outlined in the [build instructions](https://github.com/InfiniTimeOrg/InfiniTime/blob/main/doc/buildAndProgram.md)

#### Script

The dev environment comes with some scripts to make this easier, They are located in /opt/.

There are also VS Code tasks provided should you desire to use those.

The task "update submodules" will update the git submodules

### Build

You can use the build.sh script located in /opt/

CMake is also configured and controls for the CMake plugin are available in VS Code

### Debugging

Docker on windows does not support passing USB devices to the underlying WSL2 subsystem, To get around this we use OpenOCD in server mode running on the host.

`openocd -f <yourinterface> -f <nrf52.cfg target file>`

This will launch OpenOCD in server mode and attach it to the MCU.

The default launch.json file expects OpenOCD to be listening on port 3333, edit if needed

## Current Issues

Currently WSL2 Has some real performance issues with IO on a windows host. Accessing files on the virtualized filesystem is much faster. Using VS Codes "clone in container" feature of the Remote - Containers will get around this. After the container is built you will need to update the submodules and follow the build instructions like normal
