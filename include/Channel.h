#pragma once

#include <functional>

#include "Epoll.h"

class Epoll;
class Channel {
 public:
  Channel(Epoll *epoll, int fd);

  void handle_read_event();

  void handle_write_event();

  void handle_disconnect();

  void enable_read();

  void enable_write();

  void disable_write();

  void useET();

  bool inepoll() const;
  void atepoll();

  int fd() const;

  uint32_t events() const;

  uint32_t reevents() const;
  void reevents(uint32_t ev);

  void read_event_callback(std::function<void()> function);

  void write_event_callback(std::function<void()> function);

  void disconnect_callback(std::function<void()> function);

 private:
  Epoll *epoll_;
  bool inepoll_;
  int fd_;
  uint32_t events_;
  uint32_t reevents_;
  std::function<void()> read_event_callback_;
  std::function<void()> write_event_callback_;
  std::function<void()> disconnect_callback_;
};