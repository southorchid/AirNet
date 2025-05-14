#include "Server.h"

Server::Server(const std::string& host, int port, Driver::MODEL model,
               int reactor_count,
               std::function<void(std::shared_ptr<Connection>)> func)
    : host_(host),
      port_(port),
      reactor_count_(reactor_count),
      main_reactor_(std::make_shared<EventLoop>(model)),
      acceptor_(std::make_unique<Acceptor>(main_reactor_)),
      thread_pool_(std::make_unique<ThreadPool>(reactor_count_)),
      onconnect_callback_(func) {
  acceptor_->newconnect_callback(
      [this](int fd, std::unique_ptr<InetAddress> address) {
        this->newconnect(fd, std::move(address));
      });
  acceptor_->bind(host_, port_);
  acceptor_->listen();

  for (int i = 0; i < reactor_count_; ++i) {
    auto reactor = std::make_shared<EventLoop>(model);
    sub_reactor_.emplace_back(reactor);
    thread_pool_->enqueue([reactor]() { reactor->run(); });
  }
}

void Server::run() { main_reactor_->run(); }

void Server::onconnect_callback(
    std::function<void(std::shared_ptr<Connection>)> func) {
  onconnect_callback_ = func;
}

void Server::newconnect(int fd, std::unique_ptr<InetAddress> address) {
  Log::info("Client {} connected from {}:{}", fd, address->host(),
            address->port());
  std::shared_ptr<Connection> conn = std::make_shared<Connection>(
      sub_reactor_[fd % reactor_count_], fd, std::move(address));

  // 设置上层业务处理的回调函数
  conn->onconect_callback([this](std::shared_ptr<Connection> connection) {
    this->onconnect_callback_(connection);
  });

  // 设置处理连接断开的回调函数
  conn->disconnect_callback([this, fd]() { this->disconnect(fd); });

  // 将新连接添加到连接集合中（std::unordered_map<int,std::unique_ptr<Connection>>）
  std::lock_guard<std::mutex> lock(mutex_);
  connections_.insert(std::make_pair(fd, std::move(conn)));
}

void Server::disconnect(int fd) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = connections_.find(fd);
  if (it == connections_.end()) {
    return;
  }
  connections_.erase(fd);
  Log::info("Client {} disconnected", fd);
}
