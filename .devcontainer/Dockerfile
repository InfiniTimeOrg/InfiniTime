# devcontainer-specific docker container.
#
# Infinitime has a "build" container which is created by the files in
# /docker/build/. This container can be built locally and is also built
# automatically by the github workflow.
#
# We essentially "wrap" this main docker image so that devcontainers
# can make any customisations required without duplicating the main
# dockerfile or altering it in ways that might break the CI/CD build
# system.

# You can use your own locally-built infinitime-build container (see
# `/docker/README.md), or you can let it download the latest container
# from dockerhub.

FROM infinitime/infinitime-build:latest
#FROM infinitime-build:latest

# Add the infinitime user so that devcontainer can set our UID
# to ensure bind mounts have the right permissions on them.
# Also create and chown the mountpoints.
RUN \
    adduser infinitime && \
    mkdir -p /home/infinitime/.vscode-server/extensions && \
    mkdir -p /home/infinitime/.vscode-server-insiders/extensions && \
    chown -R infinitime:infinitime /home/infinitime/.vscode* && \
    # Get required vars into the local env at "login"
    echo "source /opt/build.sh" >> /home/infinitime/.bashrc && \
    # Undo the exit-on-error and PS4-echo flags set in build.sh
    echo "set +e" >> /home/infinitime/.bashrc && \
    echo "set +x" >> /home/infinitime/.bashrc && \
    # symlink the crosscompiler path so that the intellisense extension
    # can find it in a predictable location. Future builder containers
    # will already have this link. FIXME: This is very hacky and should be
    # removed once fixed in the published image.
    if  [ ! -h /opt/gcc-arm-none-eabi ]; then ln -s $(ls -d1 /opt/gcc-arm-none-eabi-* | tail -1 ) /opt/gcc-arm-none-eabi; fi && \
    if  [ ! -h /opt/nRF5_SDK          ]; then ln -s $(ls -d1 /opt/nRF5_SDK_*          | tail -1 ) /opt/nRF5_SDK; fi

# Setup envars used in vscode settings to symlinks created by build.sh in the build container
ENV NRF5_SDK_PATH /opt/nRF5_SDK
ENV ARM_NONE_EABI_TOOLCHAIN_PATH /opt/gcc-arm-none-eabi
ENV SOURCES_DIR /sources

USER infinitime

RUN bash -c "source /opt/build.sh; "

CMD ["/opt/build.sh"]
