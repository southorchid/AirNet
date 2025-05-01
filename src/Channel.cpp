#include "Channel.h"

Channel::Channel(std::shared_ptr<EventLoop> loop, int fd)
    : loop_(loop),
      inepoll_(false),
      fd_(fd),
      events_(0),
      reevents_(0),
      close_(false) {}

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
    loop_->del(fd_);
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
  loop_->update(this);
}

void Channel::disable_read() {
  if (events_ & EPOLLIN) {
    events_ &= ~EPOLLIN;
    loop_->update(this);
  }
}

void Channel::close_read() {
  if (close_read_callback_) {
    close_read_callback_();
  } else {
    Log::error("Channel {} not set close read function", fd_);
  }
}

void Channel::enable_write() {
  if (events_ & EPOLLOUT) {
    return;
  }
  events_ |= EPOLLOUT;
  loop_->update(this);
}

void Channel::disable_write() {
  if (events_ & EPOLLOUT) {
    events_ &= ~EPOLLOUT;
    loop_->update(this);
  }
}

void Channel::close_write() {
  if (close_write_callback_) {
    close_write_callback_();
  } else {
    Log::error("Channel {} not set close write function", fd_);
  }
}

void Channel::useET() {
  if (events_ & EPOLLET) {
    return;
  }
  events_ |= EPOLLET;
  loop_->update(this);
}

bool Channel::is_inepoll() const { return inepoll_; }

void Channel::inepoll() { inepoll_ = true; }

int Channel::fd() const { return fd_; }

uint32_t Channel::events() const { return events_; }

uint32_t Channel::reevents() const { return reevents_; }

bool Channel::is_close() const { return close_; }

void Channel::close() { close_ = true; }

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

void Channel::close_read_callback(std::function<void()> func) {
  close_read_callback_ = func;
}

void Channel::close_write_callback(std::function<void()> func) {
  close_write_callback_ = func;
}
