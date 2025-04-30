#pragma once

#include <functional>

#include "Epoll.h"

class Epoll;
class Channel {
 public:
  Channel(Epoll *epoll, int fd);

  void handle_read_event();

  void enable_read();

  bool inepoll() const;
  void atepoll();

  int fd() const;

  uint32_t events() const;

  uint32_t reevents() const;
  void reevents(uint32_t ev);

  void handle_read_event_function(std::function<void()> function);

 private:
  Epoll *epoll_;
  bool inepoll_;
  int fd_;
  uint32_t events_;
  uint32_t reevents_;
  std::function<void()> handle_read_event_function_;
};