#!/bin/bash
dir="$(dirname $(dirname $(dirname $(dirname $(readlink -f $0)))))"
cd $dir

docker_build="util/docker_build.sh"

[ -x "$docker_build" ] && $docker_build handwired/lian_test:default
