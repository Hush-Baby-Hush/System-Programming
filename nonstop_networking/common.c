/**
 * nonstop_networking
 * CS 241 - Fall 2021
 */
#include "common.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
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
    // Your Code Here

    size_t result = 0;
    while (result < count) {
      ssize_t read_ = read(socket, buffer + result, count - result);
      if (read_ == -1 && errno == EINTR) continue;
      if (read_ == 0) break;
      if (read_ == -1) return -1;
      result += read_;
    }
    return result;
}

ssize_t write_to_socket(int socket, const char *buffer, size_t count) {
    size_t result = 0;
    while (result < count) {
      ssize_t write_ = write(socket, buffer + result, count - result);
      if (write_ == -1 && errno == EINTR) continue;
      if (write_ == 0) break;
      if (write_ == -1) return -1;
      result += write_;
    }
    return result;
}
