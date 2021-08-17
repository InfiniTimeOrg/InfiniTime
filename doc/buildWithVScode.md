# Build and Develop the project using VS Code

The .VS Code folder contains configuration files for developing InfiniTime with VS Code. Effort was made to have these rely on Environment variables instead of hardcoded paths.

## Environment Setup

To support as many setups as possible the VS Code configuration files expect there to be certain environment variables to be set.

 Variable | Description | Example
----------|-------------|--------
**ARM_NONE_EABI_TOOLCHAIN_PATH**|path to the toolchain directory|`export ARM_NONE_EABI_TOOLCHAIN_PATH=/opt/gcc-arm-none-eabi-9-2020-q2-update`
**NRF5_SDK_PATH**|path to the NRF52 SDK|`export NRF5_SDK_PATH=/opt/nRF5_SDK_15.3.0_59ac345`

## VS Code Extensions

We leverage a few VS Code extensions for ease of development.

#### Required Extensions

- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) - C/C++ IntelliSense, debugging, and code browsing.
- [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) - Extended CMake support in Visual Studio Code

#### Optional Extensions

[Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug) - ARM Cortex-M GDB Debugger support for VS Code

Cortex-Debug is only required for interactive debugging using VS Codes built in GDB support.



## VS Code/Docker DevContainer

The .devcontainer folder contains the configuration and scripts for using a Docker dev container for building InfiniTime

Using the [Remote-Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension is recommended. It will handle configuring the Docker virtual machine and setting everything up. 

More documentation is available in the [readme in .devcontainer](.devcontainer/readme.md)	





