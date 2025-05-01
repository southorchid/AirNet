#pragma once

#include <memory>
#include <unordered_map>

#include "Acceptor.h"
#include "Connection.h"
#include "EventLoop.h"

class Server {
 public:
  static Server& instance(
      const std::string& host, int port,
      std::function<void(std::shared_ptr<Connection>)> func);

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  void run();

  void onconnect_callback(
      std::function<void(std::shared_ptr<Connection>)> func);

 private:
  Server(const std::string& host, int port);

  void newconnect(int fd, std::unique_ptr<InetAddress> address);

  void disconnect(int fd);

 private:
  std::string host_;
  int port_;
  std::shared_ptr<EventLoop> loop_;
  std::unique_ptr<Acceptor> acceptor_;
  std::unordered_map<int, std::shared_ptr<Connection>> connections_;
  std::function<void(std::shared_ptr<Connection>)> onconnect_callback_;
};