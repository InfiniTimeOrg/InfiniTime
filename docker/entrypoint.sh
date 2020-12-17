#!/bin/bash
set -e
## Create a user on-the-fly before running CMD
## This allows us to override at runtime, allowing use of a pre-built docker image
addgroup --gid $GROUP_ID user
adduser --disabled-password --gecos '' --uid $USER_ID --gid $GROUP_ID user
exec gosu user:user /bin/bash -c "$@"