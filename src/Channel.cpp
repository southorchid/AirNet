#include "Channel.h"

Channel::Channel(Epoll* epoll, int fd)
    : epoll_(epoll), inepoll_(false), fd_(fd), events_(0), reevents_(0) {}

void Channel::handle_read_event() {
  if (read_event_callback_) {
    read_event_callback_();
  } else {
    Log::warn("Channel {} not set handle read event function", fd_);
  }
}

void Channel::handle_write_event() {
  if (write_event_callback_) {
    write_event_callback_();
  } else {
    Log::warn("Channel {} not set handle write event function", fd_);
  }
}

void Channel::handle_disconnect() {
  if (disconnect_callback_) {
    disconnect_callback_();
  } else {
    Log::warn("Channel {} not set handle disconnect function", fd_);
  }
}

void Channel::enable_read() {
  if (events_ & EPOLLIN) {
    return;
  }
  events_ |= EPOLLIN;
  epoll_->update(this);
}

void Channel::enable_write() {
  if (events_ & EPOLLOUT) {
    return;
  }
  events_ |= EPOLLOUT;
  epoll_->update(this);
}

void Channel::disable_write() {
  if (events_ & EPOLLOUT) {
    events_ &= ~EPOLLOUT;
    epoll_->update(this);
  }
}

void Channel::useET() {
  if (events_ & EPOLLET) {
    return;
  }
  events_ |= EPOLLET;
  epoll_->update(this);
}

bool Channel::inepoll() const { return inepoll_; }

void Channel::atepoll() { inepoll_ = true; }

int Channel::fd() const { return fd_; }

uint32_t Channel::events() const { return events_; }

uint32_t Channel::reevents() const { return reevents_; }

void Channel::reevents(uint32_t ev) { reevents_ = ev; }

void Channel::read_event_callback(std::function<void()> function) {
  read_event_callback_ = function;
}

void Channel::write_event_callback(std::function<void()> function) {
  write_event_callback_ = function;
}

void Channel::disconnect_callback(std::function<void()> function) {
  disconnect_callback_ = function;
}
