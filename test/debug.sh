#！/bin/bash
ulimit -c unlimited
sysctl -w kernel.core_pattern=/corefile/core.%e.%p.%s.%E