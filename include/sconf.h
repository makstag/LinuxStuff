#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <limits.h>


#define BUF_SIZE 210000
#define SOCK_PATH "/tmp/saddr"
#define LEN 1024

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 199309
#include <signal.h>

using Clock = std::chrono::steady_clock;
using namespace std::chrono;
using namespace std::literals;

extern void perror(const char *__s);
extern void exit(int status);
