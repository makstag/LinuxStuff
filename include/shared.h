#ifndef SHARED_H_
#define SHARED_H_

#define __STDC_WANT_LIB_EXT1__ 1
#define _BSD_SOURCE

#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "tlpi_hdr.h"

#define IPC 1
#define SNAME "/shm"
#define LENGTH 16

typedef struct SHARED
{
    int val;
    char msg[LENGTH];

    sem_t sem;
} shared;

#endif // SHARED_H_