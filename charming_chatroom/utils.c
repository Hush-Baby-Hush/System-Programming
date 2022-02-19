/**
 * charming_chatroom
 * CS 241 - Fall 2021
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    // Your code here
    ssize_t si = htonl(size);
    return write_all_to_socket(socket, (char*) &si , 4);
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    ssize_t n = 0;
    while (n < (ssize_t) count) {
        ssize_t re = read(socket, buffer + n, count - n);
        if (re == 0) {
            return 0;
        } else if (re > 0) {
            n += re;
        } else if (re == -1 && errno != EINTR) {
            return -1;
        }
    }
    return n;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    ssize_t n = 0;
    while (n < (ssize_t) count) {
        ssize_t wr = write(socket, buffer + n, count - n);
        if (wr == 0) {
            return 0;
        } else if (wr > 0) {
            n += wr;
        } else if (wr == -1 && errno != EINTR) {
            return -1;
        }
    }
    return n;
}
