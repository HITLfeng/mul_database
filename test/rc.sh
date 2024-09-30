#! /bin/sh


pkill kvserver
ipcrm -a
source ../scripts/env.sh

cd -
sh build.sh clean
sh build.sh
cd -

echo "======================"
echo "gdb --args ./kv-test --gtest_filter=*"
echo "======================"