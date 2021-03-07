#! /bin/bash

source $(dirname $0)/../target.sh

# Start the jlink gdb server
JLinkGDBServer -if swd -device $SOC -speed auto
