# Build the project using Docker
There are [Docker images (Dockerfile)](../docker) containing the build environment for AMD64 (x86_64) and ARM64 architectures. These images make the build of the firmware and the generation of the DFU file for OTA quite easy, as well as preventing clashes with any other toolchains or development environments you may have installed.

Based on Ubuntu 18.04 with the following build dependencies:

When this is done, a new image named *infinitime-build* is available.
* ARM GCC Toolchain
* nRF SDK
* MCUBoot
* adafruit-nrfutil

## Run a container to build the project:
The `infinitime-build` image contains all the dependencies you need. The default `CMD` will compile sources found in `/sources`, so you need only mount your code. 

This example  will build the firmware, generate the MCUBoot image and generate the DFU file. Outputs will be written to **<project_root>/build/output**:

```bash
$ cd <project_root> # e.g. cd ./work/Pinetime
$ docker run --rm -it -v $(pwd):/sources infinitime-build
```

* `--rm` to delete the container after we're done. Build cache will be written out to host disk, so no worries.
* `-it` for better interactivity with the running container. 
    * `-i` Interactive - required if you want to send CTRL+C
    * `-t` TTY - This lets colours work nicely etc?
* `-v` for mounting a volume to the container

Output files (and the cmake build cache) will have `uid`/`gid` not matching your host user. To override them, use `docker run` like this:

```bash
$ docker run --rm -it -v $(pwd):/sources \
    -e USER_ID=$(id -u) -e GROUP_ID=$(id -g) \
    infinitime-build
```

If you only want to build a single CMake target, you can pass it in as the first parameter to the build script. This means calling the script explicitly as it will override the `CMD`. Here's an example For `pinetime-app`:

```bash
$ docker run --rm -it -v $(pwd):/sources infinitime-build /opt/build.sh pinetime-app
```

## Build the image
The image is not (yet) available on DockerHub, you need to build it yourself, but that is quite easy. The following commands must be run from the root of the project. This operation will take some time but, when done, a new image named *infinitime-build* is available.

```bash
$ docker image build -t infinitime-build ./docker
```

The resulting

You can bake your custom USER_ID and GROUP_ID numbers in to the image, then you don't have to pass them to `docker run` every time.

```bash
docker image build -t infinitime-build \
    --build-arg USER_ID=$(id -u) \
    --build-arg GROUP_ID=$(id -g) \
    ./docker
```
