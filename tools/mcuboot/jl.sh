#!/bin/bash

source $(dirname $0)/../target.sh

JLinkExe -speed auto -si SWD -device $SOC
