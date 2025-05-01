#pragma once

#include <functional>
#include <memory>

#include "Buffer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Log.h"
#include "Socket.h"

class Connection : public std::enable_shared_from_this<Connection> {
 public:
  enum STATE { CONNECT = 0, READCLOSE, WRITECLOSE, DISCONNECT };
  Connection(EventLoop *loop, int fd, std::unique_ptr<InetAddress> address);

  void read();

  template <class T>
  void write(T &&t);

  void write();

  void close_read();

  void close_write();

  const Buffer &read_buffer() const;

  STATE state();

  void onconect_callback(std::function<void(std::shared_ptr<Connection>)> func);

  void disconnect_callback(std::function<void()> func);

 private:
  static const int DATA_MAX_SIZE = 1024;
  EventLoop *loop_;
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  std::unique_ptr<InetAddress> address_;
  std::unique_ptr<Buffer> read_buffer_;
  std::unique_ptr<Buffer> write_buffer_;
  STATE state_;
  std::function<void(std::shared_ptr<Connection>)> onconect_callback_;
  std::function<void()> disconnect_callback_;
};

template <class T>
inline void Connection::write(T &&t) {
  std::ostringstream oss;
  oss << t;
  write_buffer_->append(oss.str());
  write();
}
