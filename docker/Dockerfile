FROM ubuntu:18.04

RUN \
    apt-get update -qq && \
    apt-get install -y \
# x86_64 / generic packages
      bash git gosu \
      cmake make build-essential \
      wget unzip \
      python3 python3-pip \ 
# aarch64 packages
      libffi-dev libssl-dev python3-dev \ 
    && rm -rf /var/lib/apt/lists/*;

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

# Set and arg and use it in the env for power to override at build AND runtime
ARG USER_ID=33333
ARG GROUP_ID=33333
ENV USER_ID $USER_ID
ENV GROUP_ID $GROUP_ID

ENV SOURCES_DIR /sources
COPY entrypoint.sh /opt/
ENTRYPOINT ["/opt/entrypoint.sh"]
CMD ["/opt/build.sh"]
