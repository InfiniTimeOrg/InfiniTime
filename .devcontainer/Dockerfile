# devcontainer-specific docker container.
#
# We essentially "wrap" the main docker image so that devcontainers
# can make any customisations required without affecting the main
# dockerfile (which the CI/CD system might rely on), and to avoid
# duplicating the main docker file (and maintaining that copy).

# You can let the local system build your own infinitime-build container,
# or you can have it download one from the infinitime repo on dockerhub.

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
    if  [ ! -h /opt/gcc-arm-none-eabi ]; then ln -s $(ls -d1 /opt/gcc-arm-none-eabi-* | tail -1 ) /opt/gcc-arm-none-eabi; fi


USER infinitime

RUN bash -c "source /opt/build.sh; "

CMD ["/opt/build.sh"]
