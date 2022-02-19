// author: zl32

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>


// connect to (host, port)
int connect_host(const char* host, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
    struct hostent* hostent = gethostbyname(host);
    if (!hostent) {
        return -1;
    }
    memcpy(&sockaddr.sin_addr.s_addr, hostent->h_addr, hostent->h_length);
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    int ret = connect(fd, (struct sockaddr*)(&sockaddr), sizeof(sockaddr));
    if (ret < 0) {
        return -1;
    }
    return fd;
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s host begin_port end_port\n", argv[0]);
        return EXIT_FAILURE;
    }

    int bport = atoi(argv[2]);
    int eport = atoi(argv[3]);
    const char request[] = "GET / HTTP/1.0\r\n\r\n";
    char buff[1024];
    int nrequest = strlen(request);
    for (int i = bport; i < eport; i++) {
        int fd = connect_host(argv[1], i);
        int found = 0;
        if (fd > 0) {
            if (send(fd, request, nrequest, 0) > 0) {
                memset(buff, 0, 1024);
                recv(fd, buff, 1023, 0);
                // printf("buff=%s\n", buff);
                if (strncmp(buff, "HTTP/1.0 200 OK", strlen("HTTP/1.0 200 OK")) == 0) {
                    found = 1;
                }
            }
            close(fd);
        }
        if (found) {
            printf("%d", i);
            break;
        } else {
            printf(".");
        }
        fflush(stdout);
    }
    printf("\n");
    return 0;
}
