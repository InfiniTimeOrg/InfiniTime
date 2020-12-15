# Build the project using Docker
There are [Docker images (Dockerfile)](../docker) containing the build environment for AMD64 (x86_64) and ARM64 architectures. These images make the build of the firmware and the generation of the DFU file for OTA quite easy, as well as preventing clashes with any other toolchains or development environments you may have installed.

## Using the image from Docker Hub

The image is avaiable via Docker Hub for both the amd64 and arm64v8 architectures at [pfeerick/infinitime-build](https://hub.docker.com/repository/docker/pfeerick/infinitime-build). 

It can be pulled (downloaded) using the following command:

```
docker pull pfeerick/infinitime-build
```

The default `latest` tag *should* automatically identify the correct image architecture, but if for some reason Docker does not, you can specify it manually:

* For AMD64 (x86_64) systems: `docker pull pfeerick/infinitime-build:amd64`

* For ARM64v8 (ARM64/aarch64) systems: `docker pull pfeerick/infinitime-build:arm64v8`

The Docker Hub images are built using 1000:1000 for the user id and group id. If this is different to your user or group ids (run `id -u` and `id -g` to find out what your id values are if you are unsure), you will need to override them via the `--user` parameter in order to prevent permission errors during and after compilation.

The below example will run the container, setting the user and group ids automatically:

```
docker run --rm -v <project_root>:/sources --user $(id -u):$(id -g) pfeerick/infinitime-build
```

Or you can specify your user id and group id (by number, not by name) directly:

```
docker run --rm -v <project_root>:/sources --user uid_num:gid_num pfeerick/infinitime-build
```

## Build the image yourself
Building the docker images yourself is quite easy. The following commands must be run from the root of the project.

The `PUID` and `PGID` build arguments are used to set the user and group ids used in the container, meaning you will not need to specify it later unless they change for some reason. Specifying them is not mandatory, as this can be over-ridden at build time via the `--user` flag, but doing so will make the command you need to run later a bit shorter. In the below examples, they are set to your current user id and group id automatically. You can specify them manually, but they must be specified by number, not by name.

If you are running on a AMD64 (x86_64) computer: 
```
docker image build -t infinitime-build --build-arg PUID=$(id -u) --build-arg PGID=$(id -g) docker/amd64/
```

If you are running on an ARM64 computer (tested on Raspberry Pi 4 and Pine64 Pinebook Pro):
```
docker image build -t infinitime-build --build-arg PUID=$(id -u) --build-arg PGID=$(id -g) docker/arm64v8/
```

This operation will take some time, as it builds a Docker image based on Ubuntu, installs some required packages, downloads the ARM toolchain, the NRF SDK, MCUBoot and adafruit-nrfutil.

When this is done, a new image named *infinitime-build* is available.

## Run a container to build the project:

The command to run the container is essentially the same, regardless of whether you built it yourself from the dockerfiles, or are using the Docker Hub images (use `pfeerick/infinitime-build` instead of `infinitime-build` for the later):

```
docker run --rm -v <project_root>:/sources infinitime-build
```

This will start a container (removing it when finished), build the firmware and generate the MCUBoot image and DFU file. The output of the build is stored in `<project_root>/built/output`.

Replace *<project_root>* by the path of the root of the project on your computer. For example:

```
docker run --rm -v /home/jf/git/PineTime:/sources infinitime-build
```

If you encounter permission errors (due to being logged in as a different user, changed user id, running the docker hub image, etc.), see the `--user` parameter mentioned above in the Docker Hub image section to see if this resolves the issue for you.
