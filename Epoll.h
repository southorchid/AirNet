#pragma once

#include <string.h>
#include <sys/epoll.h>

#include <vector>

#include "Log.h"

class Epoll {
 public:
  Epoll();

  std::vector<int> wait(int timeout = -1);

  void add(int fd) const;

  void mod(int fd) const;

  void del(int fd) const;

 private:
  static constexpr int EVENTS_MAX_SIZE = 1024;
  int epfd_;
  epoll_event events_[EVENTS_MAX_SIZE];
};