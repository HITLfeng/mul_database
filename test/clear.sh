#! /bin/bash

set -x
pkill gdb
pkill kvserver
ipcrm -a
# kill -9 `pidof kvserver`
source ../scripts/env.sh
set +x
