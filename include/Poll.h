#pragma once

#include <poll.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "Channel.h"
#include "Driver.h"
#include "Log.h"

class Poll : public Driver {
 public:
  Poll();
  void multiplexing(int timeout = -1) override;

  void update(Channel* ch) override;

  void del(int fd) override;

  std::vector<struct pollfd> poll(int timeout = -1);

 private:
  static const int FDS_MAX_SIZE = 1024;
  struct pollfd fds_[FDS_MAX_SIZE];
  int max_index_;
  std::unordered_map<int, Channel*> channels_;
};