#pragma once

#include <string.h>
#include <sys/select.h>

#include <vector>

#include "Log.h"

class Select {
 public:
  Select();

  std::vector<int> wait(int timeout = -1);

  void add(int fd);

  void del(int fd);

 private:
  fd_set fdset_;
  int maxfd_;
};