#pragma once

#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "Log.h"

class Socket {
 public:
  Socket();
  explicit Socket(int fd);

  ~Socket();

  int fd() const;

  void close();

 private:
  int fd_;
};