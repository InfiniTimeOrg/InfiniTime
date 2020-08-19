# Build the project using Docker
A [Docker image (Dockerfile)](../docker) containing all the build environment is available for X86_64 and AMD64 architectures. This image makes the build of the firmware and the generation of the DFU file for OTA.

## Build the image
The image is not (yet) available on DockerHub, you need to build it yourself, which is quite easy. The following commands must be run from the root of the project.

If you are running on a x86_64 computer : 
```
docker image build -t infinitime-build docker/x86_64/
```

And if your are running on an ARM64 device (tested on RaspberryPi4 and Pine64 PineBookPro):
```
docker image build -t infinitime-build docker/arm64/
```

This operation will take some time. It builds a Docker image based on Ubuntu, install some packages, download the ARM toolchain, the NRF SDK, MCUBoot and adafruit-nrfutil.

When this is done, a new image named *infinitime-build* is available.

## Run a container to build the project:

```
docker run --rm -v <project_root>:/sources infinitime-build
```

Replace *<project_root>* by the path of the root of the project on your computer. For example:

```
docker run --rm -v /home/jf/git/PineTime:/sources infinitime-build
```

This will start a container, build the firmware and generate the MCUBoot image and the DFU file. The output of the build is stored in **<project_root>/built/output**.