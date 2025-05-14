#pragma once

#include <string.h>
#include <sys/select.h>

#include <unordered_map>
#include <vector>

#include "Channel.h"
#include "Driver.h"
#include "Log.h"

class Select : public Driver {
 public:
  Select();

  void multiplexing(int timeout = -1) override;

  void update(Channel *ch) override;

  void del(int fd) override;

  std::vector<int> select(int timeout = -1);

 private:
  std::unordered_map<int, Channel *> channels_;
  int maxfd_;
  fd_set readfds_;
  fd_set writefds_;
  fd_set exceptfds_;
  fd_set readtemp_;
  fd_set writetemp_;
  fd_set excepttemp_;
};