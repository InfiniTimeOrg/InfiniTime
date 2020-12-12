FROM arm64v8/ubuntu:18.04

RUN apt-get update -qq && apt-get install -y wget unzip cmake make build-essential git python3 python3-pip libffi-dev libssl-dev python3-dev
RUN wget -q https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-aarch64-linux.tar.bz2 -O - | tar -xj -C /opt/
RUN wget -q https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.3.0_59ac345.zip && unzip -q nRF5_SDK_15.3.0_59ac345.zip -d /opt/ && rm nRF5_SDK_15.3.0_59ac345.zip

RUN git clone https://github.com/JuulLabs-OSS/mcuboot.git /opt/mcuboot && pip3 install -r /opt/mcuboot/scripts/requirements.txt
RUN pip3 install adafruit-nrfutil

ARG PUID=1000
ARG PGID=1000
RUN groupadd --system --gid $PGID infinitime && useradd --system --uid $PUID --gid $PGID infinitime

USER infinitime:infinitime
CMD ["/sources/docker/build.sh"]
