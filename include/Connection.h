#pragma once

#include <functional>
#include <memory>

#include "Buffer.h"
#include "Epoll.h"
#include "InetAddress.h"
#include "Log.h"
#include "Socket.h"

class Connection {
 public:
  enum STATE { CONNECT = 0, DISCONNECT };
  Connection(Epoll *epoll, int fd, std::unique_ptr<InetAddress> address);

  const std::vector<char> &read();

  void write(std::vector<char> data);
  void write();

  const std::vector<char> read_buffer() const;

  void append(std::vector<char> data);

  STATE state();

  void handle_onconnect_function(std::function<void(Connection *)> function);

  void handle_disconnect_function(std::function<void()> function);

 private:
  static const int DATA_MAX_SIZE = 1024;
  Epoll *epoll_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  std::unique_ptr<InetAddress> address_;
  std::unique_ptr<Buffer> read_buffer_;
  std::unique_ptr<Buffer> write_buffer_;
  STATE state_;
  std::function<void(Connection *)> handle_onconnect_functioon_;
  std::function<void()> handle_disconnect_function_;
};