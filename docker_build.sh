#! /bin/bash

usage() { echo "
Usage: $0 [OPTIONS]

-j Use JLink
-o Use OpenOCD
-f Force image rebuild
-c <string> Specify GDB Client e.g '/dev/ttyACM0'" 1>&2;
	exit 1;}

FORCE_REBUILD=0
while getopts "jofc:" option; do
case ${option} in
	j )
		USE_JLINK=1
	;;
	o )
		USE_OPENOCD=1
	;;
	c )
		GDB_CLIENT=${OPTARG}
	;;
	f )
		FORCE_REBUILD=1
	;;
	\? )
		usage
	;;
esac
done

docker container ls 1>&2 2>/dev/null
if [ $? -gt 0 ]
then
	echo "
	You do not have access to the docker daemon.
	Make sure your user is a member of the docker group
	or run this script as root
	" 
	exit
fi

docker image ls | grep "gcc-nrf-arm"
if [ $FORCE_REBUILD ]  || [ $? -gt 0 ]
then
	echo "Building docker image"
	docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) ./docker -t gcc-nrf-arm:2019q3
fi


docker run --rm \
	-v `pwd`:/pinetime \
	-e USE_JLINK=${USE_JLINK:-0} \
	-e USE_OPENOCD=${USE_OPENOCD:-0} \
	-e GDB_CLIENT=$GDB_CLIENT \
	gcc-nrf-arm:2019q3
