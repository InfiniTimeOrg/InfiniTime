FROM gitpod/workspace-full

USER root
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
# aarch64 packages
      libffi-dev \
      libssl-dev \
      python3-dev \ 
    && rm -rf /var/cache/apt/* /var/lib/apt/lists/*;

# Needs to be installed as root
RUN pip3 install adafruit-nrfutil

COPY docker/build.sh /opt/
# Lets get each in a separate docker layer for better downloads
# GCC
RUN bash -c "source /opt/build.sh; GetGcc;"
# NrfSdk
RUN bash -c "source /opt/build.sh; GetNrfSdk;"
# McuBoot
RUN bash -c "source /opt/build.sh; GetMcuBoot;"

# Link the default checkout workspace in to the default $SOURCES_DIR
RUN ln -s /workspace/Pinetime /sources

USER gitpod