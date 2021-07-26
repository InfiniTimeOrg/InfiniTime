# VScode devcontainer
This is a docker-based interactive development environment using VSCode and Docker Devcontainers removing the need to install any tools locally*



## Requirements

- VScode
  - [Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension
- Docker
- OpenOCD - For debugging

## Using

### Code editing, and building.

1. Clone InifiniTime and update submodules
2. Launch VSCode
3. Open InifiTime directory, 
4. Allow VSCode to open folder with devcontainer. 

After this the environment will be built if you do not currently have a container setup, it will install all the necessary tools and extra VSCode extensions. 

In order to build InfiniTime we need to run the initial submodule init and Cmake commands.

#### Manually 

 You can use the VSCode terminal to run the CMake commands as outlined in the [build instructions](blob/develop/doc/buildAndProgram.md) 

#### Script

The dev enviroment comes with some scripts to make this easier, They are located in /opt/.

There are also VSCode tasks provided should you desire to use those. 

The task "update submodules" will update the git submodules



### Build

You can use the build.sh script located in /opt/

CMake is also configured and controlls for the CMake plugin are available in VSCode



### Debugging

Docker on windows does not support passing USB devices to the underlying WSL2 subsystem, To get around this we use openocd in server mode running on the host.

`openocd -f <yourinterface> -f <nrf52.cfg target file>`

This will launch openocd in server mode and attach it to the MCU. 

The default launch.json file expects openocd to be listening on port 3333, edit if needed


## Current Issues
Currently WSL2 Has some real performance issues with IO on a windows host. Accessing files on the virtualized filesystem is much faster. Using VSCodes "clone in container" feature of the Remote - Containers will get around this. After the container is built you will need to update the submodules and follow the build isntructions like normal 