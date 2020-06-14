FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -q -y wget unzip tar cmake make build-essential && apt clean all

RUN wget -q https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/8-2019q3/RC1.1/gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2
RUN wget -q https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5/Binaries/nRF5SDK153059ac345.zip

RUN unzip -q nRF5SDK153059ac345.zip -d /opt
RUN tar -xjf gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2 -C /opt

RUN mkdir /build /source

COPY build.sh /build.sh

WORKDIR /build
CMD /build.sh

