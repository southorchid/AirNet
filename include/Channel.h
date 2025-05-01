#pragma once

#include <functional>

#include "EventLoop.h"

class EventLoop;
class Channel {
 public:
  Channel(std::shared_ptr<EventLoop> loop, int fd);

  void handle_read_event();

  void handle_write_event();

  void handle_disconnect();

  void enable_read();

  void disable_read();

  void close_read();

  void enable_write();

  void disable_write();

  void close_write();

  void useET();

  bool is_inepoll() const;
  void inepoll();

  int fd() const;

  uint32_t events() const;

  uint32_t reevents() const;
  void reevents(uint32_t ev);

  bool is_close() const;
  void close();

  void read_event_callback(std::function<void()> function);

  void write_event_callback(std::function<void()> function);

  void disconnect_callback(std::function<void()> function);

  void close_read_callback(std::function<void()> func);

  void close_write_callback(std::function<void()> func);

 private:
  std::shared_ptr<EventLoop> loop_;
  bool inepoll_;
  int fd_;
  uint32_t events_;
  uint32_t reevents_;
  bool close_;
  std::function<void()> read_event_callback_;
  std::function<void()> write_event_callback_;
  std::function<void()> disconnect_callback_;
  std::function<void()> close_read_callback_;
  std::function<void()> close_write_callback_;
};