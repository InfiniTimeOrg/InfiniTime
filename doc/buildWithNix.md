# Build the project using `nix-shell`
A `shell.nix` file is included in the repository to set up the development environment on systems that have the [Nix package manager](https://nixos.org/) installed.
It prevents your system from getting polluted with build dependencies while also guaranteeing a reproducible build environment.
All dependencies and tools required to build InfiniTime will automatically be fetched for you.

## Install Nix

Nix is available for Linux, MacOS and WSL2.
[Visit the official website for instructions on how to install it.](https://nixos.org/download#download-nix)

## Clone the repository

Before building, local repository must be fully initialized.

```sh
git clone https://github.com/InfiniTimeOrg/InfiniTime.git
cd InfiniTime
git submodule update --init
```

## Enter the development shell

Creating and entering the development environment is as easy as typing the following command in the project root:
```sh
nix-shell --run "$SHELL"
```

## Build the project
Two steps are required to build the project: `cmake` and `make`.
The `shell.nix` file provides a custom script called `cmake_infinitime` that calls `cmake` with the proper paths for the compiler and the SDK.

Inside the shell you can build a target like this:
```sh
cmake_infinitime
make pinetime-app
```

You can pass additional arguments to the `cmake` script:
```sh
cmake_infinitime -DBUILD_RESOURCES=1`
```

To make use of all the cores your host machine has pass the `-j` argument to `make`:
```sh
make -j$(nproc) pinetime-app
```

There are more targets and configuration options available.
You can find them in [`buildAndProgram.md`](./buildAndProgram.md).
