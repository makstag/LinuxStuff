#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <chrono>

// #define BUF_SIZE 1073741824
#define BUF_SIZE 1000
#define SOCK_PATH "/tmp/saddr"

using Clock = std::chrono::steady_clock;
using namespace std::chrono;
using namespace std::literals;

extern void perror(const char *__s);
extern void exit(int status);
