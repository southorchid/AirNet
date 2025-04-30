#include "Epoll.h"

Epoll::Epoll() : epfd_(-1), events_{} {
  epfd_ = epoll_create(EPOLL_CLOEXEC);
  if (epfd_ == -1) {
    Log::fatal("Epoll create failed: {}", strerror(errno));
    throw std::runtime_error("Epoll create failed");
  }
}

std::vector<int> Epoll::wait(int timeout) {
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
  std::vector<int> fds;
  fds.reserve(infds);
  for (int i = 0; i < infds; ++i) {
    fds.push_back(events_[i].data.fd);
  }
  return fds;
}

void Epoll::add(int fd) const {
  epoll_event ev;
  ev.data.fd = fd;
  ev.events = EPOLLIN;
  if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    Log::error("Epoll ctl add {} failed: {}", fd, strerror(errno));
  }
}

void Epoll::mod(int fd) const {
  epoll_event ev;
  ev.data.fd = fd;
  ev.events = EPOLLIN;
  if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    Log::error("Epoll ctl add {} failed: {}", fd, strerror(errno));
  }
}

void Epoll::del(int fd) const {
  if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    Log::error("Epoll ctl del {} failed: {}", fd, strerror(errno));
  }
}
