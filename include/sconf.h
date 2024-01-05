#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <errno.h>
#include "error_functions.h"

#define BUF_SIZE 1073741824
#define SOCK_PATH "/tmp/saddr"
