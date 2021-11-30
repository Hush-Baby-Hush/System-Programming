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


char **parse_args(int argc, char **argv);
verb check_args(char **args);

//functions
int connect_to_server( char *host,  char *port);
int read_response(verb method);
int handle_put();

static char** args;
static int sock_fd;

int main(int argc, char **argv) {
    // Good luck!
    args = parse_args(argc, argv);

    verb method = check_args(args);

    if (method == V_UNKNOWN) {
        exit(1);
    }

    char *host = args[0];
    char *port = args[1];
    sock_fd = connect_to_server(host, port);
    if (sock_fd == 1) {
        exit(1);
    }

    char* temp;
    int flag = 0;
    if (method == LIST) {
        temp = malloc(strlen(args[2]) + 2);
        sprintf(temp, "%s\n", args[2]);
    }

    else {
        size_t total_len = strlen(args[2])+strlen(args[3])+3;
        temp = malloc(total_len);
        sprintf(temp, "%s %s\n", args[2], args[3]);
    }

    int count_wr = write_to_socket(sock_fd, temp, (ssize_t)strlen(temp));
    if (count_wr < (ssize_t)strlen(temp)) {
        print_connection_closed();
        flag = 1;
    }

    free(temp);

    if (flag == 1) {
        exit(1);
    }

    if (read_response(method) == 1) {
        exit(1);
    }

    shutdown(sock_fd, SHUT_RD);

    close(sock_fd);
    free(args);
    return 0;
}

int connect_to_server( char *host,  char *port) {
    struct addrinfo hint, *result;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    int addrinfo_ = getaddrinfo(host, port, &hint, &result);
    if (addrinfo_) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addrinfo_));
        return 1;
    }
    int sockt_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sockt_fd == -1) {
        perror("socket()");
        return 1;
    }
    int connection = connect(sockt_fd, result->ai_addr, result->ai_addrlen);
    if (connection == -1) {
        perror("connect()");
        return 1;
    }
    freeaddrinfo(result);
    return sock_fd;
}

int read_response(verb method) {
    if (method == PUT) {
        handle_put();
    }
    
    if (shutdown(sock_fd, SHUT_WR) != 0) {
       perror("shutdown()");
       return 1;
    }

    char* ok = "OK\n";
    char* err = "ERROR\n";
    char* res = calloc(1, strlen(ok)+1);
    size_t num_read = read_from_socket(sock_fd, res, strlen(ok));


    if (strcmp(res, ok)) {
        char* new_buffer = realloc(res, strlen(err) + 1);
        read_from_socket(sock_fd, new_buffer + num_read, strlen(err) - num_read);
        
        if (!strcmp(new_buffer, err)) {
            char message[24] = {0};
            if (!read_from_socket(sock_fd, message, 24)) {
                print_connection_closed();
            }
            print_message(message);

        } else {
            print_invalid_response();
        }
        return 1;
    }
    
    if (method == DELETE || method == PUT) {
            print_success();
    }

    else if (method == GET) {
        FILE *local_file = fopen(args[4], "a+");
        if (!local_file) {
            perror("fopen()");
            return 1;
        }
        size_t size;
        read_from_socket(sock_fd, (char *)&size, sizeof(size_t));
        size_t rtotal = 0;
        size_t r_size;

        while (rtotal < size+4) {
            if ((size + 4 - rtotal) > 1024){
                r_size = 1024;
            }else{
                r_size = size + 4 - rtotal;
            }
            char buffer[1025] = {0};
            size_t count = read_from_socket(sock_fd, buffer, r_size);
            fwrite(buffer, 1, count, local_file);
            rtotal += count;
            if (count == 0) break;
        }

        if (rtotal < size) {
            print_too_little_data();
            return 1;
        } else if (rtotal > size) {
            print_received_too_much_data();
            return 1;
        } else if (!rtotal && rtotal != size) {
            print_connection_closed();
            return 1;
        } 
        fclose(local_file);
    }

    else if (method == LIST) {
            size_t size;
            read_from_socket(sock_fd, (char*)&size, sizeof(size_t));
            char buffer[size+6];
            memset(buffer, 0, size+6);
            num_read = read_from_socket(sock_fd, buffer, size+5);
            if (!num_read && num_read != size) {
                print_connection_closed();
                return 1;
            } else if (num_read < size) {
                print_too_little_data();
                return 1;
            } else if (num_read > size) {
                print_received_too_much_data();
                return 1;
            }
            fprintf(stdout, "%s\n", buffer);   
    }

    free(res);
    return 0;
}

int handle_put(){
    struct stat s;
    if(stat(args[4], &s) == -1) {
        return 1;
    }
    size_t size = s.st_size;
    write_to_socket(sock_fd, (char*)&size, sizeof(size_t));
    FILE* local = fopen(args[4], "r");
    if(!local) {
        return 1;
    }

    ssize_t w_count;
    size_t w_total = 0;

    while (w_total < size) {
        if ((size - w_total) <= 1024 ){
            w_count = size - w_total;
        } else {
            w_count = 1024;
        }

        char buffer[w_count + 1];
        fread(buffer, 1, w_count, local);
        ssize_t num_write = write_to_socket(sock_fd, buffer, w_count);
        if ( num_write < w_count) {
            print_connection_closed();
            return 1;
        }
        w_total += w_count;
    }

    fclose(local);
    return 0;
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
