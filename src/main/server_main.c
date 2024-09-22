
#include "main_worker.h"

int main(int argc, char *argv[])
{
    normal_info("start kv database server.");
    log_info("start kv database server and this is a info log test.");
    Status ret = GMERR_OK;
    ret = MainWorkerStart();
    return ret;
}