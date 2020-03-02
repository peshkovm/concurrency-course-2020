#!/usr/bin/env bash

if [[ $EUID == 0 ]]; then
   echo "This script must be run as non-root user inside docker group"
   exit 1
fi

if [[ -z $tpcc_docker_start_directory ]]; then
    tpcc_docker_start_directory=/tpcc/tpcc-course-2020/tasks/
fi

docker exec -it --user $(id -u):$(id -g) tpcc-image /bin/bash -c \
"if [[ -d $tpcc_docker_start_directory ]]; then
    cd $tpcc_docker_start_directory
fi
/bin/bash"
