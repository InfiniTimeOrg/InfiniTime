# Solidna baza z apt: Debian bookworm
FROM debian:bookworm

SHELL ["/bin/bash", "-lc"]

# Narzędzia systemowe + CMake + Ninja + Python + Node (do lvgl fontów)
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates build-essential git wget curl unzip xz-utils \
    cmake ninja-build make python3 python3-pip nodejs npm \
    && rm -rf /var/lib/apt/lists/*

# ARM GCC (dokładnie ta wersja, której używa InfiniTime CI)
ENV GCC_ARM_VER=10.3-2021.10
ENV GCC_ARM_DIR=/opt/gcc-arm-none-eabi-${GCC_ARM_VER}
RUN cd /opt && \
    wget -q https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 && \
    tar -xf gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2 && \
    rm gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
ENV PATH="${GCC_ARM_DIR}/bin:${PATH}"

# nRF5 SDK 15.3.0 (pakujemy do /opt, nazwa dokładnie jak w CMake)
# Uwaga: jeśli link u Ciebie wymaga logowania, podmień na własne mirror/artefakt.
ENV NRF_SDK_VER=nRF5_SDK_15.3.0_59ac345
ENV NRF5_SDK_PATH=/opt/${NRF_SDK_VER}
RUN cd /opt && \
    wget -q --no-check-certificate https://github.com/polaris-embedded/mirrors/releases/download/nrf5sdk/${NRF_SDK_VER}.zip && \
    unzip -q ${NRF_SDK_VER}.zip && rm ${NRF_SDK_VER}.zip

# Dodatkowe narzędzia używane przez InfiniTime (DFU, mcuboot imgtool, lv_font_conv)
RUN pip3 install --no-cache-dir adafruit-nrfutil==0.5.3 imgtool==1.10.0 \
    && npm install -g lv_font_conv

# Ścieżki na zewnątrz
ENV ARM_NONE_EABI_TOOLCHAIN_PATH=${GCC_ARM_DIR}
ENV NRF5_SDK_PATH=${NRF5_SDK_PATH}

WORKDIR /src
# Domyślna powłoka przy docker run
CMD ["/bin/bash"]
