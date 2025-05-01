#include "EventLoop.h"

EventLoop::EventLoop() : epoll_(std::make_unique<Epoll>()) {}

void EventLoop::run() {
  while (true) {
    auto channels = epoll_->wait();
    for (auto ch : channels) {
      int fd = ch->fd();
      uint32_t reevents = ch->reevents();
      if (reevents & EPOLLERR) {
        ch->handle_disconnect();
        break;
      }
      if (reevents & EPOLLHUP) {
        ch->handle_disconnect();
        break;
      }
      if (reevents & EPOLLPRI) {
        ch->handle_read_event();
        if (ch->is_close()) {
          break;
        }
      }
      if (reevents & EPOLLIN) {
        ch->handle_read_event();
        if (ch->is_close()) {
          break;
        }
      }
      if (reevents & EPOLLOUT) {
        ch->handle_write_event();
        if (ch->is_close()) {
          break;
        }
      }
      if (reevents & EPOLLRDHUP) {
        ch->close_read();
        break;
      }
    }
  }
}

void EventLoop::update(Channel* ch) { epoll_->update(ch); }

void EventLoop::del(int fd) { epoll_->del(fd); }