#include "log.h"
#include <fcntl.h> // 包含fcntl.h头文件
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#define MAX_CALLBACKS 32

#define LOG_USE_COLOR 1

FILE *g_logfile = NULL;
bool g_isFileInit = false;

typedef struct {
    log_LogFn fn;
    void *udata;
    int level;
} Callback;

static struct {
    void *udata;
    log_LockFn lock;
    int level;
    bool quiet;
    Callback callbacks[MAX_CALLBACKS];
} L;

static const char *level_strings[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"};
#endif

// TODO: 关闭文件描述符
void InitLogFileFd() {
    if (g_isFileInit) {
        return;
    }
    const char *filename = "/root/db/mul_database/test/sdv/log/kvserver.log";
    if (mkdir("/root/db/mul_database/test/sdv/log", 0755) == -1 && errno != EEXIST) {
        perror("无法创建目录");
        assert(false);
    }
    g_logfile = fopen(filename, "a+");
    assert(g_logfile != NULL);
    g_isFileInit = true;
}

// void Write2File(void *udata) {
//     assert(g_logfile != NULL);
//     fwrite(udata, 1, strlen((char *)udata), g_logfile);
//     fflush(g_logfile);
// }

static void stdout_callback(log_Event *ev) {
    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
#ifdef LOG_USE_COLOR
    fprintf(ev->udata, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ", buf, level_colors[ev->level],
            level_strings[ev->level], ev->file, ev->line);
#else
    fprintf(ev->udata, "%s %-5s %s:%d: ", buf, level_strings[ev->level], ev->file, ev->line);
#endif
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    // char filePrintBuf[128];

    fprintf(g_logfile, "%s %-5s %s:%d: ", buf, level_strings[ev->level], ev->file, ev->line);

    vfprintf(g_logfile, ev->fmt, ev->ap);
    fprintf(g_logfile, "\n");
    fflush(g_logfile);

    // Write2File(ev->udata);
    fflush(ev->udata);
}

static void file_callback(log_Event *ev) {
    char buf[64];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
    fprintf(ev->udata, "%s %-5s %s:%d: ", buf, level_strings[ev->level], ev->file, ev->line);
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

static void lock(void) {
    if (L.lock) {
        L.lock(true, L.udata);
    }
}

static void unlock(void) {
    if (L.lock) {
        L.lock(false, L.udata);
    }
}

const char *log_level_string(int level) { return level_strings[level]; }

void log_set_lock(log_LockFn fn, void *udata) {
    L.lock = fn;
    L.udata = udata;
}

void log_set_level(int level) { L.level = level; }

void log_set_quiet(bool enable) { L.quiet = enable; }

int log_add_callback(log_LogFn fn, void *udata, int level) {
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!L.callbacks[i].fn) {
            L.callbacks[i] = (Callback){fn, udata, level};
            return 0;
        }
    }
    return -1;
}

int log_add_fp(FILE *fp, int level) { return log_add_callback(file_callback, fp, level); }

static void init_event(log_Event *ev, void *udata) {
    if (!ev->time) {
        time_t t = time(NULL);
        ev->time = localtime(&t);
    }
    ev->udata = udata;
}

void log_log(int level, const char *file, int line, const char *fmt, ...) {
    InitLogFileFd();
    log_set_level(0); // info以上才答应
    log_Event ev = {
        .fmt = fmt,
        .file = file,
        .line = line,
        .level = level,
    };
    // const char *filename = "/root/db/mul_database/test/sdv/log/kvserver.log";
    lock();
    // if (mkdir("/root/db/mul_database/test/sdv/log", 0755) == -1 && errno != EEXIST) {
    //     perror("无法创建目录");
    //     assert(false);
    // }
    // int fd = open("filename", O_CREAT | O_RDWR | O_APPEND, 0644);
    // FILE *fp = fopen(filename, "a+");
    // assert(fp != NULL);
    // if (fd == -1) {
    //     perror("open");
    //     exit(EXIT_FAILURE);
    // }
    // log_add_fp(fp, LOG_INFO);
    if (!L.quiet && level >= L.level) {
        init_event(&ev, stderr);
        va_start(ev.ap, fmt);
        stdout_callback(&ev);
        va_end(ev.ap);
    }

    for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
        Callback *cb = &L.callbacks[i];
        if (level >= cb->level) {
            init_event(&ev, cb->udata);
            va_start(ev.ap, fmt);
            cb->fn(&ev);
            va_end(ev.ap);
        }
    }
    // fclose(fp);
    unlock();
}
