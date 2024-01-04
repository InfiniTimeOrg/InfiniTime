FROM ubuntu:latest

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
      python3-pil \
      tar \
      unzip \
      wget \ 
      curl \
      dos2unix \
      clang-format-12 \
      clang-tidy \
      locales \
      libncurses5 \
# aarch64 packages
      libffi-dev \
      libssl-dev \
      python3-dev \ 
      rustc \
    && rm -rf /var/cache/apt/* /var/lib/apt/lists/*;
    
#SET LOCALE 
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8 

RUN pip3 install adafruit-nrfutil
# required for McuBoot
RUN pip3 install setuptools_rust

WORKDIR /opt/
# build.sh knows how to compile but it problimatic on Win10
COPY build.sh .
RUN chmod +x build.sh
# create_build_openocd.sh uses cmake to crate to build directory
COPY create_build_openocd.sh .
RUN chmod +x create_build_openocd.sh
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
RUN git clone https://github.com/mcu-tools/mcuboot.git
RUN pip3 install -r ./mcuboot/scripts/requirements.txt

RUN adduser infinitime

ENV NRF5_SDK_PATH /opt/nRF5_SDK_15.3.0_59ac345
ENV ARM_NONE_EABI_TOOLCHAIN_PATH /opt/gcc-arm-none-eabi-9-2020-q2-update
ENV SOURCES_DIR /workspaces/InfiniTime
