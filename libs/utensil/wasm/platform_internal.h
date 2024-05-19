#ifndef _PLATFORM_INTERNAL_H
#define _PLATFORM_INTERNAL_H

#include "FreeRTOS.h"
#include "stdio.h"
#include "task.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef pthread_t korp_tid;
typedef pthread_mutex_t korp_mutex;
typedef pthread_cond_t korp_cond;
typedef pthread_t korp_thread;
typedef unsigned int korp_sem;

typedef int os_file_handle;
typedef int os_raw_file_handle;
typedef void *os_dir_stream;

typedef struct {
    int dummy;
} korp_rwlock;


int printf(const char *__restrict __format, ...);

static inline os_file_handle os_get_invalid_handle()
{
    return -1;
}

#endif