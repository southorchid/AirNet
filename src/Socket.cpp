#include "Socket.h"

#include "Channel.h"

Socket::Socket() : fd_(-1), reuse_(false) {
  fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (fd_ == -1) {
    Log::fatal("Create socket failed: {}", strerror(errno));
    throw std::runtime_error("Socket create failed");
  }
}

Socket::Socket(int fd) : fd_(fd), reuse_(false) {}

Socket::~Socket() { this->close(); }

void Socket::nonblock() {
  int flags = fcntl(fd_, F_GETFL, 0);
  if (flags == -1) {
    Log::error("Failed to get flags for socket {}: {}", fd_, strerror(errno));
    return;
  }
  if (flags & O_NONBLOCK) {
    Log::info("Socket {} is already in non-blocking mode.", fd_);
    return;
  }
  if (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
    Log::error("Failed to set non-blocking mode for socket {}: {}", fd_,
               strerror(errno));
  }
}

void Socket::reuse() {
  if (reuse_) {
    return;
  }
  int opt = 1;
  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    Log::error("Failed to set SO_REUSEADDR on socket {}: {}", fd_,
               strerror(errno));
    return;
  }
  if (setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
    Log::error("Failed to set SO_REUSEPORT on socket {}: {}", fd_,
               strerror(errno));
    return;
  }
  reuse_ = true;
}

int Socket::fd() const { return fd_; }

void Socket::close() {
  if (fd_ != -1) {
    ::close(fd_);
    fd_ = -1;
  }
}
