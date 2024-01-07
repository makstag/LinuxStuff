#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

// #define BUF_SIZE 1073741824
#define BUF_SIZE 1000
#define SOCK_PATH "/tmp/saddr"

extern void perror(const char *__s);
extern void exit(int status);
