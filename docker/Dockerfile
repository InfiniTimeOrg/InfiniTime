FROM debian:10.4 

ARG UNAME=user
ARG UID=1000
ARG GID=1000

COPY checksums.md5 .

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && \
    apt install -y build-essential cmake wget unzip && \
    rm -rf /var/lib/apt/lists && \
    wget "https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/8-2019q3/RC1.1/gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2" && \
    wget "https://www.nordicsemi.com/-/media/Software-and-other-downloads/SDKs/nRF5/Binaries/nRF5SDK153059ac345.zip" && \
    md5sum -c checksums.md5 && \
    tar -xf gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2 -C /opt && \
    rm gcc-arm-none-eabi-8-2019-q3-update-linux.tar.bz2 && \
    unzip nRF5SDK153059ac345.zip -d /opt && \
    rm nRF5SDK153059ac345.zip

COPY entrypoint.sh .

RUN groupadd -g $GID -o $UNAME
RUN useradd -m -u $UID -g $GID -o -s /bin/bash $UNAME
USER $UNAME

ENTRYPOINT /entrypoint.sh
