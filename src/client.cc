#include "sconf.h"

int main(void)
{
    struct sockaddr_un svaddr, claddr;
    int sfd;
    char resp[BUF_SIZE];

    /* Create client socket; bind to unique pathname (based on PID) */

    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sfd == -1)
        perror("socket");

    memset(&claddr, 0, sizeof(struct sockaddr_un));
    claddr.sun_family = AF_UNIX;
    snprintf(claddr.sun_path, sizeof(claddr.sun_path),
            "/tmp/saddr.%ld", (long) getpid());

    if (bind(sfd, (struct sockaddr *) &claddr, sizeof(struct sockaddr_un)) == -1)
        perror("bind");

    /* Construct address of server */

    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, SOCK_PATH, sizeof(svaddr.sun_path) - 1);

    /* Send messages to server; echo responses on stdout */

    /* May be longer than BUF_SIZE */
    memset(resp, '0', BUF_SIZE);
    if (sendto(sfd, resp, BUF_SIZE, 0, (struct sockaddr *) &svaddr,
            sizeof(struct sockaddr_un)) != BUF_SIZE)
    {
        printf("sendto");
        exit(EXIT_FAILURE);
    }

//    numBytes = recvfrom(sfd, resp, BUF_SIZE, 0, NULL, NULL);
    /* Or equivalently: numBytes = recv(sfd, resp, BUF_SIZE, 0);
            or: numBytes = read(sfd, resp, BUF_SIZE); */
//    if (numBytes == -1)
//        perror("recvfrom");
//    printf("Response %d: %.*s\n", j, (int) numBytes, resp);

    remove(claddr.sun_path);            /* Remove client socket pathname */
    exit(EXIT_SUCCESS);
}
