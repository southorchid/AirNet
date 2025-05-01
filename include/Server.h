#pragma once

#include <memory>
#include <unordered_map>

#include "Acceptor.h"
#include "Connection.h"
#include "EventLoop.h"
#include "ThreadPool.h"

class Server {
 public:
  Server(const std::string& host, int port, int reactor_count);
  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  void run();

  void onconnect_callback(
      std::function<void(std::shared_ptr<Connection>)> func);

 private:
  void newconnect(int fd, std::unique_ptr<InetAddress> address);

  void disconnect(int fd);

 private:
  std::string host_;
  int port_;
  int reactor_count_;
  std::shared_ptr<EventLoop> main_reactor_;
  std::vector<std::shared_ptr<EventLoop>> sub_reactor_;
  std::unique_ptr<Acceptor> acceptor_;
  std::unique_ptr<ThreadPool> thread_pool_;
  std::unordered_map<int, std::shared_ptr<Connection>> connections_;
  std::function<void(std::shared_ptr<Connection>)> onconnect_callback_;
};