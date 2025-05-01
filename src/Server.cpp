#include "Server.h"

Server& Server::instance(
    const std::string& host, int port,
    std::function<void(std::shared_ptr<Connection>)> func) {
  static Server instance(host, port);
  instance.onconnect_callback(func);
  return instance;
}

void Server::run() { loop_->run(); }

void Server::onconnect_callback(
    std::function<void(std::shared_ptr<Connection>)> func) {
  onconnect_callback_ = func;
}

Server::Server(const std::string& host, int port)
    : host_(host),
      port_(port),
      loop_(std::make_shared<EventLoop>()),
      acceptor_(std::make_unique<Acceptor>(loop_.get())) {
  acceptor_->newconnect_callback(
      [this](int fd, std::unique_ptr<InetAddress> address) {
        this->newconnect(fd, std::move(address));
      });
  acceptor_->bind(host_, port_);
  acceptor_->listen();
}

void Server::newconnect(int fd, std::unique_ptr<InetAddress> address) {
  Log::info("Client {} connected from {}:{}", fd, address->host(),
            address->port());
  std::shared_ptr<Connection> conn =
      std::make_shared<Connection>(loop_.get(), fd, std::move(address));

  // 设置上层业务处理的回调函数
  conn->onconect_callback([this](std::shared_ptr<Connection> connection) {
    this->onconnect_callback_(connection);
  });

  // 设置处理连接断开的回调函数
  conn->disconnect_callback([this, fd]() { this->disconnect(fd); });

  // 将新连接添加到连接集合中（std::unordered_map<int,std::unique_ptr<Connection>>）
  connections_.insert(std::make_pair(fd, std::move(conn)));
}

void Server::disconnect(int fd) {
  auto it = connections_.find(fd);
  if (it == connections_.end()) {
    return;
  }
  loop_->del(fd);
  connections_.erase(fd);
  Log::info("Client {} disconnected", fd);
}
