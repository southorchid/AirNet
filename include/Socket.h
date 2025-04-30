#pragma once

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "Log.h"

class Socket {
 public:
  Socket();
  explicit Socket(int fd);
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;
  ~Socket();

  void nonblock();

  void reuse();

  int fd() const;

  void close();

 private:
  int fd_;
  bool reuse_;
};