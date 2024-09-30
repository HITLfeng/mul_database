#！/bin/bash

# 获取脚本所在的目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 获取脚本的全路径
SCRIPT_PATH="$(readlink -f "${BASH_SOURCE[0]}")"

# echo "脚本所在的目录：$SCRIPT_DIR"
# echo "脚本的全路径：$SCRIPT_PATH"

# 将output/bin添加到环境变量
export PATH=$PATH:${SCRIPT_DIR}/../output/bin/

// gdb 乱码问题
echo $LANG
export LANG=en_US.UTF-8