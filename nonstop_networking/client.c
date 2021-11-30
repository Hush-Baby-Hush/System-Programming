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


void read_response(char **args, int sockt_fd, verb method);
void connect_to_server(const char *host, const char *port);

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
        temp_ = calloc(1, strlen(args[2]) + strlen(args[3]) + 3);
        sprintf(temp_, "%s %s\n", args[2], args[3]);
    }
    
    ssize_t num_wr = write_to_socket(sockt_fd, temp_, strlen(temp_));
    if (num_wr < (ssize_t)strlen(temp_)) {
        print_connection_closed();
        exit(-1);
    }

    free(temp_);

    if (method == PUT) {
        struct stat s;
        int status = stat(args[4], &s);
        if (status == -1) {
            exit(-1);
        }
        size_t size = s.st_size;
        write_to_socket(sockt_fd, (char*)&size, sizeof(size_t));
        FILE* local = fopen(args[4], "r");
        if (!local) {
            perror("fopen");
            exit(-1);
        }
        size_t num_put = 0;
        ssize_t buffer_size = 0;

        while (num_put < size) {
            if ((size - num_put) > 1024) {
                buffer_size = 1024;
            } else {
                buffer_size = size - num_put;
            }
            char buffer[buffer_size + 1];
            fread(buffer, 1, buffer_size, local);
            ssize_t num_write = write_to_socket(sockt_fd, buffer, buffer_size);
            if (num_write < buffer_size) {
                print_connection_closed();
                exit(-1);
            }
            num_put += buffer_size;
        }
        fclose(local);
    }

    int status2 = shutdown(sockt_fd, SHUT_WR);
    if (status2) perror("shutdown");

    read_response(args, sockt_fd, method);

    shutdown(sockt_fd, SHUT_RD);
    close(sockt_fd);
    free(args);
}

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

void read_response(char **args, int sockt_fd, verb method) {
    char* ok = "OK\n";
    char* err = "ERROR\n";
    char* res = calloc(1, strlen(ok)+1);
    size_t num_read = read_from_socket(sockt_fd, res, strlen(ok));

    if (!strcmp(res, ok)) {
        if (method == DELETE || method == PUT) {
            print_success();
        }else if (method == GET) {
            FILE* local_file = fopen(args[4], "a+");
            if (!local_file ) {
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
                fwrite(buffer, 1, count, local_file );
                rtotal += count;
                if (count == 0) break;
            }
            if (!rtotal && rtotal != size) {
                print_connection_closed();
                exit(-1);
            } else if (rtotal < size) {
                print_too_little_data();
                exit(-1);
            } else if (rtotal > size) {
                print_received_too_much_data();
                exit(-1);
            }
            fclose(local_file );
        }else if (method == LIST) {
            size_t size;
            read_from_socket(sockt_fd, (char*)&size, sizeof(size_t));
            char buffer[size+6];
            memset(buffer, 0, size+6);
            num_read = read_from_socket(sockt_fd, buffer, size+5);
            //error occurs
            if (!num_read && num_read != size) {
                print_connection_closed();
                exit(-1);
            } else if (num_read < size) {
                print_too_little_data();
                exit(-1);
            } else if (num_read > size) {
                print_received_too_much_data();
                exit(-1);
            }
            fprintf(stdout, "%zu%s", size, buffer);
        }
    } else {
        res = realloc(res, strlen(err)+1);
        read_from_socket(sockt_fd, res+num_read, strlen(err) - num_read);
        if (!strcmp(res, err)) {
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
