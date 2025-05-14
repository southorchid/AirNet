#include "Select.h"

#include <thread>

#include "Log.h"

Select::Select() : maxfd_(-1) {
  FD_ZERO(&readfds_);
  FD_ZERO(&writefds_);
  FD_ZERO(&exceptfds_);
  FD_ZERO(&readtemp_);
  FD_ZERO(&writetemp_);
  FD_ZERO(&excepttemp_);
}

void Select::multiplexing(int timeout) {
  std::vector<int> infds = select(1);
  if (infds.empty()) {
    return;
  }

  for (int fd : infds) {
    auto it = channels_.find(fd);
    if (it == channels_.end()) {
      Log::error("Channel {} not found", fd);
      continue;
    }
    Channel* ch = it->second;
    if (FD_ISSET(fd, &excepttemp_)) {
      Log::info("Connection {} disconnected: excepttemp", fd);
      ch->handle_disconnect();
      break;
    }
    if (FD_ISSET(fd, &readtemp_)) {
      ch->handle_read_event();
      if (ch->is_close()) {
        Log::info("Connection {} disconnected: readtemp", fd);
        ch->handle_disconnect();
        break;
      }
    }
    if (FD_ISSET(fd, &writetemp_)) {
      ch->handle_write_event();
      if (ch->is_close()) {
        Log::info("Connection {} disconnected: writetemp", fd);
        ch->handle_disconnect();
        break;
      }
    }
  }
}

void Select::update(Channel* ch) {
  int fd = ch->fd();
  if (fd == -1) {
    Log::error("Bad file descriptor");
    return;
  }

  if (FD_ISSET(fd, &exceptfds_) == 0) {
    FD_SET(fd, &exceptfds_);
    channels_[fd] = ch;
    if (fd > maxfd_) {
      maxfd_ = fd;
    }
  }

  // 对端关闭了写端，但不能将对端的fd从readfds_中移除，因为移除后就不能监视到对端断开连接的事件
  if (FD_ISSET(fd, &readfds_) == 0) {
    if (ch->enread()) {
      FD_SET(fd, &readfds_);
    }
  }

  if (FD_ISSET(fd, &writefds_) == 0) {
    if (ch->enwrite()) {
      FD_SET(fd, &writefds_);
    }
  } else {
    if (!ch->enwrite()) {
      FD_CLR(fd, &writefds_);
    }
  }
}

void Select::del(int fd) {
  FD_CLR(fd, &readfds_);
  FD_CLR(fd, &writefds_);
  FD_CLR(fd, &exceptfds_);
  auto it = channels_.find(fd);
  if (it == channels_.end()) {
    Log::error("Channel {} not found", fd);
    return;
  }
  channels_.erase(it);
  for (int i = maxfd_; i >= 0; --i) {
    if (FD_ISSET(i, &readfds_) != 0) {
      maxfd_ = i;
      break;
    }
  }
}

std::vector<int> Select::select(int timeout) {
  if (timeout < 0) {
    timeout = 1;
  }
  FD_ZERO(&readtemp_);
  FD_ZERO(&writetemp_);
  FD_ZERO(&excepttemp_);
  readtemp_ = readfds_;
  writetemp_ = writefds_;
  excepttemp_ = exceptfds_;

  int count = -1;

  // 必须启用超时机制，因为开始时没有 fd 加入，所有的 temp 都是0，不启用超时机制
  // select 会一直阻塞
  struct timeval time;
  memset(&time, 0, sizeof(time));
  time.tv_sec = timeout;
  time.tv_usec = 0;
  count = ::select(maxfd_ + 1, &readtemp_, &writetemp_, &exceptfds_, &time);

  if (count == 0) {
    // 超时
    return {};
  } else if (count == -1) {
    // 发生错误
    Log::error("Select failed: {}", strerror(errno));
    return {};
  }

  std::vector<int> infds;
  infds.reserve(count);
  for (int fd = 0; fd <= maxfd_; ++fd) {
    if (FD_ISSET(fd, &readtemp_) || FD_ISSET(fd, &writetemp_) ||
        FD_ISSET(fd, &excepttemp_)) {
      infds.push_back(fd);
    }
  }

  return infds;
}
