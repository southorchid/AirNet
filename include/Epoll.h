#pragma once

#include <string.h>
#include <sys/epoll.h>

#include <vector>

#include "Channel.h"
#include "Log.h"

class Channel;
class Epoll {
 public:
  Epoll();

  std::vector<Channel *> wait(int timeout = -1);

  void update(Channel *ch);

  void add(Channel *ch) const;

  void mod(Channel *ch) const;

  void del(Channel *ch) const;

 private:
  static constexpr int EVENTS_MAX_SIZE = 1024;
  int epfd_;
  epoll_event events_[EVENTS_MAX_SIZE];
};