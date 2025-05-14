#pragma once

#include <string.h>
#include <sys/epoll.h>

#include <vector>

#include "Channel.h"
#include "Driver.h"
#include "Log.h"

class Channel;
class Epoll : public Driver {
 public:
  Epoll();

  void multiplexing(int timeout = -1) override;

  void update(Channel *ch) override;

  void del(int fd) override;

 private:
  std::vector<Channel *> wait(int timeout = -1);

  void add(Channel *ch) const;

  void mod(Channel *ch) const;

 private:
  static constexpr int EVENTS_MAX_SIZE = 1024;
  int epfd_;
  epoll_event events_[EVENTS_MAX_SIZE];
};