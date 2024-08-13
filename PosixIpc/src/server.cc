#include "sconf.h"

static volatile int sfd;
static volatile sig_atomic_t flag;

static void handler(int /* sig */, siginfo_t * /* si */, void * /* ucontext */)
{
    flag = 1;
    struct sockaddr_un claddr;
    socklen_t len = sizeof(struct sockaddr_un);
    char buf[BUF_SIZE];

    const auto start_time = Clock::now();

    ssize_t numBytes = recvfrom(sfd, buf, BUF_SIZE, 0, (struct sockaddr *) & claddr, & len);

    const auto end_time = Clock::now();
    const auto dur = end_time -start_time;

    std::cout << "receive = " << duration_cast<milliseconds>(dur).count() / 1000.0 << " s\n";

    if (numBytes == -1)
        perror("recvfrom");

    /* std::cout << "buf = " << buf << std::endl; */
    printf("Server received %ld bytes from %s\n", (long) numBytes, claddr.sun_path);
}

int main(void)
{
    std::cout << "PID = " << getpid() << std::endl;
    flag = 0;
    struct sockaddr_un svaddr;
    struct sigaction sa;
    sigset_t origMask, blockMask;
    
    sigemptyset(& blockMask);
    sigaddset(& blockMask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, & blockMask, & origMask) == -1)
        perror("sigprocmask - SIG_BLOCK");

    sigemptyset(& sa.sa_mask);
    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;

    if (sigaction(SIGUSR1, & sa, NULL) == -1)
        perror("sigaction");

    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);       /* Create server socket */
    if (sfd == -1)
        perror("socket");

    /* Construct well-known address and bind server socket to it */

    /* For an explanation of the following check, see the erratum note for
       page 1168 at http://www.man7.org/tlpi/errata/. */

    if (strlen(SOCK_PATH) > sizeof(svaddr.sun_path) - 1)
    {
        printf("Server socket path too long: %s", SOCK_PATH);
        exit(EXIT_FAILURE);
    }

    if (remove(SOCK_PATH) == -1 && errno != ENOENT)
        perror("remove-/tmp/saddr");

    memset(&svaddr, 0, sizeof(struct sockaddr_un));
    svaddr.sun_family = AF_UNIX;
    strncpy(svaddr.sun_path, SOCK_PATH, sizeof(svaddr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un)) == -1)
        perror("bind");
    
    /* Receive messages, convert to uppercase, and return to client */

    while (not flag)
        if (sigsuspend(& origMask) == -1)
            perror("sigsuspend");

    if (sigprocmask(SIG_SETMASK, &origMask, NULL) == -1)
        perror("sigprocmask - SIG_SETMASK");

    exit(EXIT_SUCCESS);
}
