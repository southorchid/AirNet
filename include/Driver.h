#pragma once

#include <vector>

class Channel;
class Driver {
 public:
  enum MODEL { SELECT = 0, POLL, EPOLL };

  // I/O 多路复用函数
  virtual void multiplexing(int timeout = -1) = 0;

  // 更新监视的文件描述符
  virtual void update(Channel* ch) = 0;

  // 删除监视的文件描述符
  virtual void del(int fd) = 0;
};