#pragma once

#include <functional>
#include <memory>

#include "Channel.h"
#include "Epoll.h"
#include "InetAddress.h"
#include "Log.h"
#include "Socket.h"

class Acceptor {
 public:
  Acceptor(Epoll *epoll);

  void bind(const std::string &host, int port);

  void listen(int maxconn = SOMAXCONN);

  void accept();

  void handle_newconnect_function(
      std::function<void(int, std::unique_ptr<InetAddress>)> function);

 private:
  Epoll *epoll_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  std::unique_ptr<InetAddress> address_;
  std::function<void(int, std::unique_ptr<InetAddress>)>
      handle_newconnect_function_;
};