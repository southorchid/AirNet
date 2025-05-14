#pragma once

#include <memory>

#include "Channel.h"
#include "Driver.h"
#include "Epoll.h"
#include "Select.h"

class Channel;
class EventLoop {
 public:
  explicit EventLoop(Driver::MODEL model);
  void run();

  EventLoop(const EventLoop&) = delete;
  EventLoop& operator=(const EventLoop&) = delete;

  void update(Channel* ch);

  void del(int fd);

  std::unique_ptr<Driver> driver(Driver::MODEL model);

 private:
  std::unique_ptr<Driver> driver_;
};