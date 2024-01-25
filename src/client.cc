#include "sconf.h"

int main(void)
{
    struct sockaddr_un svaddr, claddr;
    int sfd;
    char resp[BUF_SIZE], line[LEN];
    union sigval sv;
    sv.sival_int = 100;

    FILE * cmd = popen("pidof server", "r");
    fgets(line, LEN, cmd);
    pid_t pid = strtoul(line, NULL, 10);
    pclose(cmd);

    std::cout << "PID = " << pid << std::endl;

    /* Create client socket; bind to unique pathname (based on PID) */

    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sfd == -1)
        perror("socket");

//    memset(&claddr, 0, sizeof(struct sockaddr_un));
//    claddr.sun_family = AF_UNIX;
//    snprintf(claddr.sun_path, sizeof(claddr.sun_path),
//            "/tmp/saddr.%ld", (long) getpid());

//    if (bind(sfd, (struct sockaddr *) &claddr, sizeof(struct sockaddr_un)) == -1)
//        perror("bind");

    /* Construct address of server */

    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, SOCK_PATH, sizeof(svaddr.sun_path) - 1);

    /* Send messages to server; echo responses on stdout */

    /* May be longer than BUF_SIZE */
    memset(resp, '0', BUF_SIZE);
//    resp[0] = 'h';
//    resp[1] = 'e';
//    resp[2] = 'l';
//    resp[3] = 'l';
//    resp[4] = 'o';

    const auto start_time = Clock::now();
    ssize_t senBuf = sendto(sfd, resp, BUF_SIZE, 0, (struct sockaddr *) & svaddr, sizeof(struct sockaddr_un));
//    std::cout << "memory initialize = " << MSG_SIZE << std::endl;

    if (errno == EMSGSIZE)
    {
        printf("sendto");
        exit(EXIT_FAILURE);
    }
    
    if (sigqueue(pid, SIGUSR1, sv) == -1)
        perror("sigqueue");

    const auto end_time = Clock::now();
    const auto dur = end_time - start_time;
    std::cout << "send = " << duration_cast<milliseconds>(dur).count() / 1000.0 << " s\n";
    std::cout << "min sig = " << SIGRTMIN << std::endl;
    std::cout << "max sig = " << SIGRTMAX << std::endl;

    remove(claddr.sun_path);            /* Remove client socket pathname */
    exit(EXIT_SUCCESS);
}
