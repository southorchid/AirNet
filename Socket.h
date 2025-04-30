#pragma once

#include <errno.h>
#include <string.h>
#include <sys/socket.h>

#include <iostream>

#include "Log.h"

class Socket {
 public:
  Socket();
  explicit Socket(int fd);

  int fd() const;

 private:
  int fd_;
};