FROM ubuntu:18.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update -qq \
    && apt-get install -y \
# x86_64 / generic packages
      bash \
      build-essential \
      cmake \
      git \
      make \
      python3 \
      python3-pip \
      tar \
      unzip \
      wget \ 
      curl \
# aarch64 packages
      libffi-dev \
      libssl-dev \
      python3-dev \ 
      rustc \
    && rm -rf /var/cache/apt/* /var/lib/apt/lists/*;

RUN pip3 install adafruit-nrfutil
# required for McuBoot
RUN pip3 install setuptools_rust

WORKDIR /opt/
# build.sh knows how to compile but it problimatic on Win10
COPY build.sh .
# create_build_openocd.sh uses cmake to crate to build directory
COPY create_build_openocd.sh .

# Lets get each in a separate docker layer for better downloads
# GCC
# RUN bash -c "source /opt/build.sh; GetGcc;"
RUN wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 -O - | tar -xj -C /opt
# NrfSdk
# RUN bash -c "source /opt/build.sh; GetNrfSdk;"
RUN wget -q "https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.3.0_59ac345.zip" -O /tmp/nRF5_SDK_15.3.0_59ac345
RUN unzip -q /tmp/nRF5_SDK_15.3.0_59ac345 -d /opt
RUN rm /tmp/nRF5_SDK_15.3.0_59ac345
# McuBoot
# RUN bash -c "source /opt/build.sh; GetMcuBoot;"
RUN git clone https://github.com/JuulLabs-OSS/mcuboot.git
RUN pip3 install -r ./mcuboot/scripts/requirements.txt

RUN adduser infinitime

ENV SOURCES_DIR /workspaces/Pinetime
