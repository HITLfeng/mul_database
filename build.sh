#！/bin/bash
source /opt/rh/devtoolset-8/enable

source ./scripts/env.sh

case "$1" in
    clean)
        # 清空目录
        echo "clearing..."
        # rm main 2>/dev/null
        rm -rf ./output/bin
        rm -rf ./output/lib
        rm -rf ./output/include
        echo "clearing done!"
        ;;
    *)
        # 执行其他操作
        mkdir -p ./output/include
        cp ./src/interface/include/* ./output/include/
        source ./innerbuild.sh
        ;;
esac


