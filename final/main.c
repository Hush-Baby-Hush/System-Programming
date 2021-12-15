
//
//  main.c
//  search
//
//  Created by Kimmy Liu on 12/15/21.
//
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<string.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

//#define PORT 12345

int main(int argc, char* argv[])
{
  struct sockaddr_in addr;
  int fd;
  char  buffer2[256];
    struct hostent *remoteHost;
    char *host_name;
    struct in_addr addr2;
    char *buffer1 = "GET / HTTP/1.0\r\n\r\n";
//    asprintf(&buffer1,
//      "GET / HTTP/1.0\r\n\r\n"
//      "Connection: close\r\n"
//      "Accept: */*\r\n\r\n");
    int port=1000;
    int con;
    int port_start = atoi(argv[2]);
    int port_end =atoi(argv[3]);
//    printf("portstart%d",port_start);
//    printf("portend%d",port_end);
//    printf("char %s",argv[1]);
    
    


  fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd == -1)
  {
      printf("Error opening socket\n");
      return -1;
  }
//    host_name = "mydemo.cs.illinois.edu";
//    host_name = "arsarabi.eecs.umich.edu";
//    host_name = "tsy19.github.io";
    host_name = argv[1];
    remoteHost = gethostbyname(host_name);
    if (remoteHost != NULL) {
        if (remoteHost->h_addrtype == AF_INET)
        {
            if (remoteHost->h_addr_list[0] != 0){
                addr2.s_addr = *(u_long *) remoteHost->h_addr_list[0];
                addr.sin_addr.s_addr = inet_addr(inet_ntoa(addr2));
//                printf("\tIP Address : %s\n", inet_ntoa(addr2));
//                printf("\tIP Address : %s\n", );
            }
        }
//        printf("Function returned:\n");
//        printf("\tOfficial name: %s\n", remoteHost->h_name);
//        for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) {
//            printf("\tAlternate name #%d: %s\n", ++i, *pAlias);
//        }
//        printf("\tAddress type: ");
//        switch (remoteHost->h_addrtype) {
//        case AF_INET:
//            printf("AF_INET\n");
//            break;
//        case AF_NETBIOS:
//            printf("AF_NETBIOS\n");
//            break;
//        default:
//            printf(" %d\n", remoteHost->h_addrtype);
//            break;
    }
//        printf("\tAddress length: %d\n", remoteHost->h_length);
    
    
    
    addr.sin_family = AF_INET;
    for(port=port_start; port<port_end; port++){
        addr.sin_port = htons(port);
        if(bind(fd, (struct sockaddr *)&addr,sizeof(struct sockaddr_in) ) == -1)
        {
//            printf("Error binding socket\n");
            printf(".");
            continue;
        }

        
        con = connect(fd, (struct sockaddr*) &addr, sizeof addr);
        if (con != 0){
//            printf("Error in Connection\n");
            printf(".");
            continue;
        }
        
//        char *buffer;
//        asprintf(&buffer,
//          "GET %s HTTP/1.0\r\n"
//          "Connection: close\r\n"
//          "Accept: */*\r\n\r\n");

        write(fd, buffer1, sizeof(buffer1));
        read(fd, buffer2, sizeof(buffer2));
//        free(buffer);
//
//        if (send(fd, buffer1, sizeof(buffer1), 0) < 0){
//            printf("send failed");
//            printf(".");
//            continue;
//        }
        
//        if (recv(fd, buffer2, 256, 0) < 0 ) {
////            fprintf(stderr, "recv: %s (%d)\n", strerror(errno), errno);
//            printf(".");
//            continue;
//        };
//        printf("send : %s\n", buffer1);
        printf("send%d:%s\n",port,buffer1);
        printf("recv%d:%s\n", port, buffer2);
        printf("%s\n", repr(buffer2));
//        break;
    }
    
//  addr.sin_port = htons(PORT);
//  addr.sin_addr.s_addr = 0;
//  addr.sin_addr.s_addr = inet_addr("141.213.13.199");
//  addr.sin_addr.s_addr = INADDR_ANY;
  


//  printf(" %s\n", addr.sin_addr.s_addr);
    
//    if (listen(fd, 3) == 0){
//        printf("Listening ...\n");
//    }
//    else {
//        printf("Unable to listen\n");
//    }
    
//    socklen_t addr_size = sizeof addr;

    
//    strcpy(buffer2, "GET / HTTP/1.0\r\n\r\n");
    
    
    
    
    
    
    
    
    
    
    return 0;
}
