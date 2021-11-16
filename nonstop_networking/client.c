/**
 * nonstop_networking
 * CS 241 - Fall 2021
 */
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

static int sockt_fd;
char **parse_args(int argc, char **argv);
verb check_args(char **args);

void connect_to_server(const char *host, const char *port) {
    struct addrinfo hint, *result;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    int addrinfo_ = getaddrinfo(host, port, &hint, &result);
    if (addrinfo_) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addrinfo_));
        exit(1);
    }
    int sockt_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sockt_fd == -1) {
        perror("socket");
        exit(1);
    }
    int connection = connect(sockt_fd, result->ai_addr, result->ai_addrlen);
    if (connection == -1) {
        perror("connect");
        exit(1);
    }
    freeaddrinfo(result);
}

int main(int argc, char **argv) {
    // Good luck!
    char** args = parse_args(argc, argv);
    verb method = check_args(args);
    char* host = args[0];
    char* port = args[1];
    connect_to_server(host, port);

    char* temp_;
    if (method == LIST) {
        temp_ = calloc(1, strlen(args[2])+2);
        sprintf(temp_, "%s\n", args[2]);   
    } else {
        temp_ = calloc(1, strlen(args[2]) + strlen(args[3])+3);
        sprintf(temp_, "%s %s\n", args[2], args[3]);
    }
    // ssize_t length = strlen(temp_);
    ssize_t wcount = write_to_socket(sockt_fd, temp_, strlen(temp_));
    if (wcount < strlen(temp_)) {
        print_connection_closed();
        exit(-1);
    }
    free(temp_);

    //if method is put
    if (method == PUT) {
        struct stat s;
        int status = stat(args[4], &s);
        if (status == -1) {
            exit(-1);
        }
        //write size
        size_t size = s.st_size;
        write_to_socket(sockt_fd, (char*)&size, sizeof(size_t));
        //write data
        FILE* local = fopen(args[4], "r");
        if (!local) {
            exit(-1);
        }
        ssize_t w_size;
        size_t wtotal = 0;
        while (wtotal < size) {
            if ((size - wtotal) > 1024) {
                w_size = 1024;
            } else {
                w_size = size - wtotal;
            }
            char buffer[w_size + 1];
            fread(buffer, 1, w_size, local);
            if (write_to_socket(sockt_fd, buffer, w_size) < w_size) {
                print_connection_closed();
                exit(-1);
            }
            wtotal += w_size;
        }
        fclose(local);
    }
    int status2 = shutdown(sockt_fd, SHUT_WR);
    if (status2 != 0) {
        perror("shutdown");
    }
    //read response from server()
    char* ok = "OK\n";
    char* err = "ERROR\n";
    char* res = calloc(1, strlen(ok)+1);
    size_t read_byte = read_from_socket(sockt_fd, res, strlen(ok));
    if (strcmp(res, ok) == 0) {
        fprintf(stdout, "%s", res);
        //delete or put response ok
        if (method == DELETE || method == PUT) {
            print_success();
        }
        if (method == GET) {
            FILE* local= fopen(args[4], "a+");
            if (!local) {
                perror("fopen");
                exit(-1);
            }
            size_t size;
            read_from_socket(sockt_fd, (char*)&size, sizeof(size_t));
            size_t rtotal = 0;
            size_t r_size;
            while (rtotal < size+5) {
                if ((size+5 -rtotal) > 1024) {
                    r_size = 1024;
                } else {
                    r_size = size+5-rtotal;
                }
                char buffer[1025] = {0};
                size_t count = read_from_socket(sockt_fd, buffer, r_size);
                fwrite(buffer, 1, count, local);
                rtotal += count;
                if (count == 0) {break;}
            }
            if (rtotal == 0 && rtotal != size) {
                print_connection_closed();
                exit(-1);
            } else if (rtotal < size) {
                print_too_little_data();
                exit(-1);
            } else if (rtotal > size) {
                print_received_too_much_data();
                exit(-1);
            }
            fclose(local);
        }
        //when method is list   
        if (method == LIST) {
            size_t size;
            read_from_socket(sockt_fd, (char*)&size, sizeof(size_t));
            char buffer[size+6];
            memset(buffer, 0, size+6);
            read_byte = read_from_socket(sockt_fd, buffer, size+5);
            //error occurs
            if (read_byte == 0 && read_byte != size) {
                print_connection_closed();
                exit(-1);
            } else if (read_byte < size) {
                print_too_little_data();
                exit(-1);
            } else if (read_byte > size) {
                print_received_too_much_data();
                exit(-1);
            }
            fprintf(stdout, "%zu%s", size, buffer);
        }
    } else {
        res = realloc(res, strlen(err)+1);
        read_from_socket(sockt_fd, res+read_byte, strlen(err) - read_byte);
        if (strcmp(res, err) == 0) {
            fprintf(stdout, "%s", res);
            char err_message[20] = {0};
            if (!read_from_socket(sockt_fd,err_message, 20)) {
                print_connection_closed();
            }
            print_error_message(err_message);
        } else {
            print_invalid_response();
        }
    }
    free(res);
    //
    shutdown(sockt_fd, SHUT_WR);
    close(sockt_fd);
    free(args);
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
