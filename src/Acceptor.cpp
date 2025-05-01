#include "Acceptor.h"

Acceptor::Acceptor(EventLoop* loop)
    : loop_(loop),
      socket_(std::make_unique<Socket>()),
      address_(nullptr),
      channel_(std::make_unique<Channel>(loop, socket_->fd())) {
  socket_->reuse();
  channel_->read_event_callback([this]() { this->accept(); });
  channel_->enable_read();
}

void Acceptor::bind(const std::string& host, int port) {
  address_ = std::make_unique<InetAddress>(host, port);
  if (::bind(socket_->fd(), address_->addr(), address_->len()) == -1) {
    Log::fatal("Acceptor bind {}:{} failed: {}", address_->host(),
               address_->port(), strerror(errno));
    throw std::runtime_error("Acceptor bind address failed");
  }
}

void Acceptor::listen(int maxconn) {
  if (address_ == nullptr) {
    Log::fatal("Acceptor not bind address");
    throw std::runtime_error("Acceptor not bind address");
  }
  if (::listen(socket_->fd(), maxconn) == -1) {
    Log::fatal("Acceptor listen failed: {}", strerror(errno));
    throw std::runtime_error("Acceptor listen failed");
  }
}

void Acceptor::accept() {
  std::unique_ptr<InetAddress> client_addr = std::make_unique<InetAddress>();
  socklen_t client_addr_len = client_addr->len();
  int clientfd = ::accept(socket_->fd(), client_addr->addr(), &client_addr_len);
  if (clientfd == -1) {
    Log::error("Acceptor accept failed: {}", strerror(errno));
    return;
  }
  if (newconnect_callback_) {
    newconnect_callback_(clientfd, std::move(client_addr));
  } else {
    Log::fatal("Acceptor not set handle new connection function");
    throw std::runtime_error("Acceptor not set handle new connect function");
  }
}

void Acceptor::newconnect_callback(
    std::function<void(int, std::unique_ptr<InetAddress>)> func) {
  newconnect_callback_ = func;
}
