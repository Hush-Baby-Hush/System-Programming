/**
 * nonstop_networking
 * CS 241 - Fall 2021
 */
#include "common.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "format.h"
#include <stdio.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>



ssize_t read_from_socket(int socket, char *buffer, size_t count) {
    size_t result = 0;
    while (result < count) {
        ssize_t read_code = read(socket, (void*) (buffer + result), count - result);
        if (read_code == -1 && errno == EINTR) continue;
        if (read_code == 0) break;
        if (read_code == -1) return 1;
        result += read_code;
    }
    return result;
}


ssize_t write_to_socket(int socket, const char *buffer, size_t count) {
    size_t result = 0;
    while (result < count) {
        ssize_t write_code = write(socket, (void*) (buffer + result), count - result);
        if (write_code == -1 && errno == EINTR) continue;
        if (write_code == 0) break;
        if (write_code == -1) return 1;
        result += write_code;
    }
    return result;
}

ssize_t read_header_from_socket(int socket, char *buffer, size_t count) {
    size_t result = 0;
    while (result < count) {
        ssize_t read_code = read(socket, (void*) (buffer + result), 1);
        if (read_code == -1 && errno == EINTR) continue;
        if (read_code == 0 || buffer[strlen(buffer) - 1] == '\n') break;
        if (read_code == -1) return 1;
        result += read_code;
    }
    return result;
}