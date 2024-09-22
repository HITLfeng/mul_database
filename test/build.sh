#！/bin/bash
source /opt/rh/devtoolset-8/enable

case "$1" in
    clean)
        # 清空目录
        echo "clearing..."
        rm main 2>/dev/null
        echo "clearing done!"
        ;;
    *)
        # 执行其他操作
        source ./innerbuild.sh
        ;;
esac

echo "./main --gtest_color=yes" --gtest_filter=* 
