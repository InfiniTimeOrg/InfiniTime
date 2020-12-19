FROM amd64/ubuntu:18.04

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update -qq \
    && apt-get install -y \ 
        build-essential \
        cmake \
        git \
        make \
        python3 \
        python3-pip \
        tar \
        unzip \
        wget \
    && rm -rf /var/cache/apt/* /var/lib/apt/lists/*

RUN wget -q https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz \
  && tar -xjf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz -C /opt \
  && rm gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz

RUN wget -q https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.3.0_59ac345.zip  \
  && unzip -q nRF5_SDK_15.3.0_59ac345.zip -d /opt/ && rm nRF5_SDK_15.3.0_59ac345.zip

RUN git clone https://github.com/JuulLabs-OSS/mcuboot.git /opt/mcuboot \
  && pip3 install -r /opt/mcuboot/scripts/requirements.txt

RUN pip3 install adafruit-nrfutil

ARG PUID=1000
ARG PGID=1000
RUN groupadd --system --gid $PGID infinitime && useradd --system --uid $PUID --gid $PGID infinitime

USER infinitime:infinitime
CMD ["/sources/docker/build.sh"]
