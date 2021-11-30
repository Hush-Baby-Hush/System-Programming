/**
 * nonstop_networking
 * CS 241 - Fall 2021
 */
#include "format.h"
#include "common.h"

#include "includes/dictionary.h"
#include "includes/vector.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <stdbool.h>

#define MAX_CLIENTS 40
#define MAX_EVENTS 50
#define TIMEOUT 50000
#define OK "OK\n"

typedef struct go_illini {
	verb command;
	char name_S[250];
    char header[1024];
    int status;
} go_illini;


static int epoll_fd;
static int sock_fd;
static dictionary* client_dir;
static dictionary* size_ser;
static vector* files_ser;
static char* _directory_;
static char* port_;

int run(go_illini* connection, int client_fd);

void close_server() {
    close(epoll_fd);
    vector_destroy(files_ser);
    dictionary_destroy(client_dir);
    dictionary_destroy(size_ser);
    exit(1);
}


void epoll_monitor(int fd) {
    struct epoll_event event;
    event.events = EPOLLOUT;  
    event.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

void handle_dir() {
    char vec[] = "XXXXXX";
    _directory_ = mkdtemp(vec);
    print_temp_directory(_directory_);
}

void connet_server() {
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd == -1) {
        perror("socket()\n");
        exit(1);
    }
    struct addrinfo hint, *result;
    memset(&hint, 0, sizeof(hint));

    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    int getaddr_ = getaddrinfo(NULL, port_, &hint, &result);
    if (getaddr_ ) {
        fprintf(stderr, "%s", gai_strerror(getaddr_));
        if (result) freeaddrinfo(result);
        exit(1);
    }
    
    int temp = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(temp))) {
        perror("setsockopt()");
        if (result) freeaddrinfo(result);
        exit(1);
    }

    if (bind(sock_fd, result->ai_addr, result->ai_addrlen) == -1) {
        perror("bind()");
        if (result) freeaddrinfo(result);
        exit(1);
    }
    
    if (listen(sock_fd, MAX_CLIENTS) == -1) {
        perror("listen()");
        if (result) freeaddrinfo(result);
        exit(1);
    }
}


void read_header(go_illini* connection, int client_fd) {
    char header[1024];
    read_header_from_socket(client_fd, header, 1024);

    if (!strncmp(header, "LIST", 4)) {
        connection->command = LIST;
    }

    else if (!strncmp(header, "PUT", 3)) {
        connection->command = PUT;
        strcpy(connection->name_S, header + 4);

    }

    else if (!strncmp(header, "GET", 3)) {
        connection->command = GET;
        strcpy(connection->name_S, header + 4);

    }

    else if (!strncmp(header, "DELETE", 6)) {
        connection->command = DELETE;
        strcpy(connection->name_S, header + 7);

    } 
    
    else {
        print_invalid_response();
        epoll_monitor(client_fd);
        return;
    }

    if (connection->command != LIST) {
        connection->name_S[strlen(connection->name_S) - 1] = '\0';
    }
    connection->status = 1;
}


void epoll_init() {
    epoll_fd = epoll_create(100);
    if (epoll_fd == -1) {
        perror("epoll_create()");
        exit(1);
    }
    struct epoll_event event;
    event.events = EPOLLIN; 
    event.data.fd = sock_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) == -1) {
        perror("epoll_ctl...()");
        exit(1);
    }
    struct epoll_event vec[MAX_EVENTS];
    while (1) {
        int count = epoll_wait(epoll_fd, vec, MAX_EVENTS, -1);
        if (count == -1) {
            perror("epoll_wait()");
            exit(1);
        } 
        
        else if (!count) continue;

        for (int i = 0; i < count; i++) {
            int temp = vec[i].data.fd;
            if (temp == sock_fd) {
                int client_fd = accept(sock_fd, NULL, NULL); 
                if (client_fd == -1) {
                    perror("accept()");
                    exit(1);
                }
                struct epoll_event event2;
                event2.events = EPOLLIN; 
                event2.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event2) == -1) {
                    perror("epoll_ctl!()");
                    exit(1);
                }
                go_illini* c = calloc(1, sizeof(go_illini));
                c->status = 0;
                dictionary_set(client_dir, &client_fd, c);
            } else {               
                go_illini* client_c = dictionary_get(client_dir, &temp);
                if (!client_c->status) {  
                    read_header(client_c, temp);
                    LOG("header read");
                } 
                else if (client_c->status == 1) { 
                    if (run(client_c, temp) != 0) {
                        //idk
                    }
                    LOG("command executed");
                }

            }
        }
    }

}

int list_luck(go_illini* connection, int client_fd) {
    LOG("execute list");

    size_t size = 0;
    VECTOR_FOR_EACH(files_ser, file, {
        size += strlen(file) + 1;
    });
    if (size) size--;

    write_all_to_socket(client_fd, (char*) &size, sizeof(size_t));
    
    VECTOR_FOR_EACH(files_ser, file, {
        write_to_socket(client_fd, file, strlen(file));
        if (_it != _iend-1) {
            write_all_to_socket(client_fd, "\n", 1);
        }
    });
    return 0;

}


int get_luck(go_illini* connection, int client_fd) {
    return 0;
}


int put_luck(go_illini* connection, int client_fd) {

	int len = strlen(_directory_) + strlen(connection->name_S) + 2;
	char vec[len];
	memset(vec , 0, len);
	sprintf(vec, "%s/%s", _directory_, connection->name_S);
    
    FILE* read_file = fopen(vec, "r");
    FILE* write_file = fopen(vec, "w");

    if (!write_file) {
        perror("fopen()");
        return 1;
    }

    size_t buff;
    read_all_from_socket(client_fd, (char*) &buff, sizeof(size_t));
    size_t b = 0;

    while (b < buff + 4) {
        size_t s;
        if ((buff + 4 - b) <= 1024) {
            s = (buff + 4 - b);
        } else {
            s = 1024;
        }
        char buff[1025];
        memset(buff, 0, 1025);
        ssize_t read_c = read_from_socket(client_fd, buff, s);
        if (read_c == -1) continue;

        fwrite(buff, 1, read_c, write_file);
        b += read_c;

        if (!read_c) break;

    }
    
    printf("FILE NAME: %s\n", connection->name_S);
    if (!read_file) {
        vector_push_back(files_ser, connection->name_S);
    } else {
        fclose(read_file);
    }
    fclose(write_file);
    dictionary_set(size_ser, connection->name_S, &buff);
    return 0;
}


int delete_luck(go_illini* connection, int client_fd) {
    return 0;
}


int run(go_illini* connection, int client_fd) {
    verb command = connection->command;
    if (command == GET) {
        get_luck(connection, client_fd);
    }
    if (command == PUT) {
        if (put_luck(connection, client_fd) != 0) {
            return 1;
        }
        write_to_socket(client_fd, OK, 3);
    }
    if (command == LIST) {
        write_to_socket(client_fd, OK, 3);
        if (list_luck(connection, client_fd) != 0) {
            return 1;
        }
    }
    if (command == DELETE) {
        if (delete_luck(connection, client_fd) != 0) {
            return 1;
        }
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);
    return 0;
}

void handle_sigint() {
    LOG("Exiting...");
    close_server();
}

int main(int argc, char **argv) {
    
    if (argc != 2){
        print_server_usage();
        exit(1);  
    }
    struct sigaction sig_;
    memset(&sig_, 0, sizeof(sig_));
    sig_.sa_handler = SIG_IGN;
    sig_.sa_flags = SA_RESTART;
    if ( sigaction(SIGPIPE, &sig_, NULL)) {
        perror("sigaction()");
        exit(1);
    }

    struct sigaction sig2_;
    memset(&sig2_, '\0', sizeof(sig2_));
    sig2_.sa_handler = handle_sigint;
    if (sigaction(SIGINT, &sig2_, NULL) == -1) {
        perror("sigaction()");
        exit(1);
    }

    handle_dir();
    LOG("directory setup");

    port_ = strdup(argv[1]);
    client_dir = int_to_shallow_dictionary_create();
    size_ser = int_to_shallow_dictionary_create();
    files_ser = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    LOG("global variables initialized");

    connet_server();
    LOG("connection setup");

    epoll_init();
    LOG("epoll setup");

}