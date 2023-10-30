FROM ubuntu:22.04

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
      python-is-python3 \
      tar \
      unzip \
      wget \
      curl \
      # aarch64 packages
      libffi-dev \
      libssl-dev \
      python3-dev \
      git \
      apt-utils \
      pkg-config \
      libpixman-1-dev \
      libcairo2-dev \
      libpango-1.0-0 \
      ibpango1.0-dev \
      libpangocairo-1.0-0 \
    && curl -sL https://deb.nodesource.com/setup_18.x | bash - \
    && apt-get install -y nodejs \
    && rm -rf /var/cache/apt/* /var/lib/apt/lists/*;

# Git needed for PROJECT_GIT_COMMIT_HASH variable setting

RUN pip3 install adafruit-nrfutil
RUN pip3 install -Iv cryptography==3.3
RUN pip3 install cbor
RUN npm i lv_font_conv@1.5.2 -g

# build.sh knows how to compile
COPY build.sh /opt/

# Lets get each in a separate docker layer for better downloads
# GCC
RUN bash -c "source /opt/build.sh; GetGcc;"
# NrfSdk
RUN bash -c "source /opt/build.sh; GetNrfSdk;"
# McuBoot
RUN bash -c "source /opt/build.sh; GetMcuBoot;"

ENV SOURCES_DIR /sources
CMD ["/opt/build.sh"]
