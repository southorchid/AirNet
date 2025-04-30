#include "Channel.h"

Channel::Channel(Epoll* epoll, int fd)
    : epoll_(epoll), inepoll_(false), fd_(fd), events_(0), reevents_(0) {}

void Channel::enable_read() {
  if (events_ & EPOLLIN) {
    return;
  }
  events_ |= EPOLLIN;
  epoll_->update(this);
}

bool Channel::inepoll() const { return inepoll_; }

void Channel::atepoll() { inepoll_ = true; }

int Channel::fd() const { return fd_; }

uint32_t Channel::events() const { return events_; }

uint32_t Channel::reevents() const { return reevents_; }

void Channel::reevents(uint32_t ev) { reevents_ = ev; }
