FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive
ARG NODE_MAJOR=20
RUN apt-get update -qq \
    && apt-get install -y ca-certificates curl gnupg \
    && mkdir -p /etc/apt/keyrings \
    && curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
    && echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_$NODE_MAJOR.x nodistro main" > /etc/apt/sources.list.d/nodesource.list \
    && apt-get update -qq \
    && apt-get install -y \
# x86_64 / generic packages
      bash \
      build-essential \
      cmake \
      git \
      make \
      nodejs \
      python3 \
      python3-pip \
      python3-pil \
      python-is-python3 \
      tar \
      unzip \
      wget \
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
