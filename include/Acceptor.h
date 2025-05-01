#pragma once

#include <functional>
#include <memory>

#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Log.h"
#include "Socket.h"

class Acceptor {
 public:
  Acceptor(std::shared_ptr<EventLoop> loop);

  void bind(const std::string &host, int port);

  void listen(int maxconn = SOMAXCONN);

  void accept();

  void newconnect_callback(
      std::function<void(int, std::unique_ptr<InetAddress>)> func);

 private:
  std::shared_ptr<EventLoop> loop_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  std::unique_ptr<InetAddress> address_;
  std::function<void(int, std::unique_ptr<InetAddress>)> newconnect_callback_;
};