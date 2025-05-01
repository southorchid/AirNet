#pragma once

#include <memory>

#include "Channel.h"
#include "Epoll.h"

class Channel;
class Epoll;
class EventLoop {
 public:
  EventLoop();
  void run();

  EventLoop(const EventLoop&) = delete;
  EventLoop& operator=(const EventLoop&) = delete;

  void update(Channel* ch);

  void del(int fd);

 private:
  std::unique_ptr<Epoll> epoll_;
};