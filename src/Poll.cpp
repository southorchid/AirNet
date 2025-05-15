#include "Poll.h"

Poll::Poll() : max_index_(-1) {
  for (auto &fd : fds_) {
    fd.fd = -1;
    fd.events = 0;
    fd.revents = 0;
  }
}

void Poll::multiplexing(int timeout) {
  auto active_fds = poll(timeout);
  if (active_fds.empty()) {
    return;
  }
  for (auto &active_fd : active_fds) {
    int fd = active_fd.fd;
    auto it = channels_.find(fd);
    if (it == channels_.end()) {
      Log::error("Channel {} not found", fd);
      continue;
    }
    auto ch = it->second;
    if (active_fd.revents & POLLERR) {
      Log::info("Connection {} disconnect: POLLERR", fd);
      ch->handle_disconnect();
      break;
    }
    if (active_fd.revents & POLLHUP) {
      Log::info("Connection {} disconnect: POLLHUP", fd);
      ch->handle_disconnect();
      break;
    }
    if (active_fd.revents & POLLIN) {
      ch->handle_read_event();
      if (ch->is_close()) {
        Log::info("Connection {} disconnect: POLLIN", fd);
        ch->handle_disconnect();
        break;
      }
    }
    if (active_fd.revents & POLLOUT) {
      ch->handle_write_event();
      if (ch->is_close()) {
        Log::info("Connection {} disconnect: POLLOUT", fd);
        ch->handle_disconnect();
        break;
      }
    }
  }
}

void Poll::update(Channel *ch) {
  int fd = ch->fd();
  if (fd == -1) {
    Log::error("Bad file descriptor");
    return;
  }
  int i = 0;  // 保存当前fd在fds_中的索引

  // 查找当前fd在fds_中的索引
  for (i = 0; i <= max_index_; ++i) {
    if (fds_[i].fd == fd) {
      break;
    }
  }

  if (!ch->is_inloop()) {
    // 如果fd不在fds_中，添加fd到fds_中
    for (i = 0; i < FDS_MAX_SIZE; ++i) {
      if (fds_[i].fd == -1) {
        // 找到一个空闲的位置
        fds_[i].fd = fd;
        fds_[i].events = 0;
        fds_[i].revents = 0;
        channels_.insert(std::make_pair(fd, ch));  // 添加到channels_中
        if (max_index_ < i) {
          max_index_ = i;
        }
        ch->inloop();  // 修改channel为已添加
        break;
      }
    }
    if (i == FDS_MAX_SIZE) {
      // 没有空闲的位置
      Log::error("No space for new fd");
      return;
    }
  }

  if (fds_[i].events & POLLIN) {
    // 已经监视了fd的读事件，判断是否需要放弃读事件
    if (!ch->enread()) {
      fds_[i].events &= ~POLLIN;
    }
  } else {
    // 没有监视fd的读事件，判断是否需要添加读事件
    if (ch->enread()) {
      fds_[i].events |= POLLIN;
    }
  }

  if (fds_[i].events & POLLOUT) {
    // 已经监视了fd的写事件，判断是否需要放弃写事件
    if (!ch->enwrite()) {
      fds_[i].events &= ~POLLOUT;
    }
  } else {
    // 没有监视fd的写事件，判断是否需要添加写事件
    if (ch->enwrite()) {
      fds_[i].events |= POLLOUT;
    }
  }
}

void Poll::del(int fd) {
  auto it = channels_.find(fd);
  if (it == channels_.end()) {
    Log::error("Channel {} not found", fd);
    return;
  }
  channels_.erase(it);
  for (int i = 0; i <= max_index_; ++i) {
    if (fds_[i].fd == fd) {
      fds_[i].fd = -1;
      fds_[i].events = 0;
      fds_[i].revents = 0;
      if (max_index_ == i) {
        // 如果删除的是最后一个fd，更新max_index_
        for (int j = max_index_ - 1; j >= 0; --j) {
          if (fds_[j].fd != -1) {
            max_index_ = j;
            break;
          }
        }
      }
      break;
    }
  }
}

std::vector<struct pollfd> Poll::poll(int timeout) {
  int count = ::poll(fds_, max_index_ + 1, 1);

  if (count == 0) {
    return {};
  }

  if (count == -1) {
    Log::error("poll error");
    return {};
  }

  std::vector<struct pollfd> active_fds;

  for (int i = 0; i <= max_index_; ++i) {
    if (fds_[i].revents != 0) {
      active_fds.emplace_back(fds_[i]);
    }
  }
  return active_fds;
}
