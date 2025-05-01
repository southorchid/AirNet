#include "Epoll.h"

Epoll::Epoll() : epfd_(-1), events_{} {
  epfd_ = epoll_create(EPOLL_CLOEXEC);
  if (epfd_ == -1) {
    Log::fatal("Epoll create failed: {}", strerror(errno));
    throw std::runtime_error("Epoll create failed");
  }
}

std::vector<Channel *> Epoll::wait(int timeout) {
  int infds = epoll_wait(epfd_, events_, EVENTS_MAX_SIZE, timeout);
  if (infds == -1) {
    // 发生错误
    Log::fatal("Epoll wait failed: {}", strerror(errno));
    return {};
  }
  if (infds == 0) {
    // 超时
    return {};
  }
  std::vector<Channel *> active_channels;
  active_channels.reserve(infds);
  for (int i = 0; i < infds; ++i) {
    Channel *ch = (Channel *)events_[i].data.ptr;
    ch->reevents(events_[i].events);
    active_channels.emplace_back(ch);
  }
  return active_channels;
}

void Epoll::update(Channel *ch) {
  if (ch->is_inepoll()) {
    // Channel已经在epoll红黑树上
    this->mod(ch);
  } else {
    this->add(ch);
  }
}

void Epoll::del(int fd) const {
  if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    Log::error("Epoll ctl del {} failed: {}", fd, strerror(errno));
  }
}

void Epoll::add(Channel *ch) const {
  int fd = ch->fd();
  if (fd == -1) {
    Log::error("Bad file descriptor {}", fd);
    return;
  }
  epoll_event ev;
  ev.data.ptr = ch;
  ev.events = ch->events();
  if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    Log::error("Epoll ctl add {} failed: {}", fd, strerror(errno));
  }
  ch->inepoll();
}

void Epoll::mod(Channel *ch) const {
  int fd = ch->fd();
  epoll_event ev;
  ev.data.ptr = ch;
  ev.events = ch->events();
  if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
    Log::error("Epoll ctl mod {} failed: {}", fd, strerror(errno));
  }
}
