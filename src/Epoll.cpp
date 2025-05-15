#include "Epoll.h"

#include <thread>

Epoll::Epoll() : epfd_(-1), events_{} {
  epfd_ = epoll_create(EPOLL_CLOEXEC);
  if (epfd_ == -1) {
    Log::fatal("Epoll create failed: {}", strerror(errno));
    throw std::runtime_error("Epoll create failed");
  }
}

void Epoll::multiplexing(int timeout) {
  auto channels = this->wait();
  for (auto ch : channels) {
    int fd = ch->fd();
    uint32_t revents = ch->revents();
    if (revents & EPOLLERR) {
      Log::info("Connection {} disconnect: {}", fd, "EPOLLERR");
      ch->handle_disconnect();
      break;
    }
    if (revents & EPOLLHUP) {
      Log::info("Connection {} disconnect: {}", fd, "EPOLLHUP");
      ch->handle_disconnect();
      break;
    }
    if (revents & EPOLLPRI) {
      ch->handle_read_event();
      if (ch->is_close()) {
        Log::info("Connection {} disconnect: {}", fd, "EPOLLPRI");
        ch->handle_disconnect();
        break;
      }
    }
    if (revents & EPOLLIN) {
      ch->handle_read_event();
      if (ch->is_close()) {
        Log::info("Connection {} disconnect: {}", fd, "EPOLLIN");
        ch->handle_disconnect();
        break;
      }
    }
    if (revents & EPOLLOUT) {
      ch->handle_write_event();
      if (ch->is_close()) {
        Log::info("Connection {} disconnect: {}", fd, "EPOLLOUT");
        ch->handle_disconnect();
        break;
      }
    }
    if (revents & EPOLLRDHUP) {
      ch->close_read();
      if (ch->is_close()) {
        Log::info("Connection {} disconnect: {}", fd, "EPOLLRDHUP");
        ch->handle_disconnect();
        break;
      }
    }
  }
}

std::vector<Channel*> Epoll::wait(int timeout) {
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
  std::vector<Channel*> active_channels;
  active_channels.reserve(infds);
  for (int i = 0; i < infds; ++i) {
    Channel* ch = (Channel*)events_[i].data.ptr;
    ch->revents(events_[i].events);
    active_channels.emplace_back(ch);
  }
  return active_channels;
}

void Epoll::update(Channel* ch) {
  if (ch->is_inloop()) {
    // Channel已经在epoll红黑树上
    this->mod(ch);
  } else {
    this->add(ch);
  }
}

void Epoll::del(int fd) {
  if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    Log::error("Epoll ctl del {} failed: {}", fd, strerror(errno));
  }
}

void Epoll::add(Channel* ch) const {
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
  ch->inloop();
}

void Epoll::mod(Channel* ch) const {
  int fd = ch->fd();
  epoll_event ev;
  ev.data.ptr = ch;
  ev.events = ch->events();
  if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
    Log::error("Epoll ctl mod {} failed: {}", fd, strerror(errno));
  }
}
