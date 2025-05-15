#include "EventLoop.h"

#include <thread>

#include "Log.h"

EventLoop::EventLoop(Driver::MODEL model) : driver_(driver(model)) {}

void EventLoop::run() {
  Log::info("Reactor {} start", std::this_thread::get_id());
  while (true) {
    driver_->multiplexing();
  }
}

void EventLoop::update(Channel* ch) { driver_->update(ch); }

void EventLoop::del(int fd) { driver_->del(fd); }

std::unique_ptr<Driver> EventLoop::driver(Driver::MODEL model) {
  switch (model) {
    case Driver::SELECT:
      return std::make_unique<Select>();
    case Driver::POLL:
      return std::make_unique<Poll>();
    case Driver::EPOLL:
      return std::make_unique<Epoll>();
    default:
      return std::make_unique<Select>();
  }
}
