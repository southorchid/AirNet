#include "Select.h"

Select::Select() : fdset_({}), maxfd_(-1) { FD_ZERO(&fdset_); }

std::vector<int> Select::wait(int timeout) {
  fd_set tmpset = fdset_;
  int count = -1;
  if (timeout <= 0) {
    // 不启用超时机制
    count = ::select(maxfd_ + 1, &tmpset, nullptr, nullptr, nullptr);
  } else {
    // 启用超时机制
    struct timeval time;
    memset(&time, 0, sizeof(time));
    time.tv_sec = timeout;
    time.tv_usec = 0;
    count = ::select(maxfd_ + 1, &tmpset, nullptr, nullptr, &time);
  }

  if (count == 0) {
    Log::info("Select timeout");
    return {};
  } else if (count == -1) {
    Log::error("Select failed: {}", strerror(errno));
    return {};
  }
  std::vector<int> infds;
  infds.reserve(count);
  for (int fd = 0; fd <= maxfd_; ++fd) {
    if (FD_ISSET(fd, &tmpset) != 0) {
      infds.emplace_back(fd);
    }
  }
  return infds;
}

void Select::add(int fd) {
  FD_SET(fd, &fdset_);
  if (maxfd_ < fd) {
    maxfd_ = fd;
  }
}

void Select::del(int fd) {
  FD_CLR(fd, &fdset_);
  for (int i = maxfd_; i >= 0; --i) {
    if (FD_ISSET(i, &fdset_) != 0) {
      maxfd_ = i;
      break;
    }
  }
}
