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

RUN adduser infinitime

RUN pip3 install adafruit-nrfutil
# required for McuBoot
RUN pip3 install setuptools_rust

WORKDIR /opt/
# build.sh knows how to compile
COPY build.sh .

# Lets get each in a separate docker layer for better downloads
# GCC
RUN bash -c "source /opt/build.sh; GetGcc;"
# NrfSdk
RUN bash -c "source /opt/build.sh; GetNrfSdk;"
# McuBoot
RUN bash -c "source /opt/build.sh; GetMcuBoot;"

ENV SOURCES_DIR /workspaces/Pinetime
