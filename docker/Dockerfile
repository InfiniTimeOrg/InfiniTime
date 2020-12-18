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
# aarch64 packages
      libffi-dev \
      libssl-dev \
      python3-dev \ 
    && rm -rf /var/cache/apt/* /var/lib/apt/lists/*;

RUN pip3 install adafruit-nrfutil

# build.sh knows how to compile
COPY build.sh /opt/

# Lets get each in a separate docker layer for better downloads
# GCC
RUN bash -c "source /opt/build.sh; GetGcc;"
# NrfSdk
RUN bash -c "source /opt/build.sh; GetNrfSdk;"
# McuBoot
RUN bash -c "source /opt/build.sh; GetMcuBoot;"

ARG PUID=1000
ARG PGID=1000
RUN groupadd --system --gid $PGID infinitime && useradd --system --uid $PUID --gid $PGID infinitime

USER infinitime:infinitime
ENV SOURCES_DIR /sources
CMD ["/opt/build.sh"]
