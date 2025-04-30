#include "Socket.h"

#include "Channel.h"

Socket::Socket() : fd_(-1) {
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ == -1) {
    Log::fatal("Create socket failed: {}", strerror(errno));
    throw std::runtime_error("Socket create failed");
  }
}

Socket::Socket(int fd) : fd_(fd) {}

Socket::~Socket() { this->close(); }

int Socket::fd() const { return fd_; }

void Socket::close() {
  if (fd_ != -1) {
    ::close(fd_);
    fd_ = -1;
  }
}
