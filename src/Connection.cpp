#include "Connection.h"

Connection::Connection(Epoll* epoll, int fd,
                       std::unique_ptr<InetAddress> address)
    : epoll_(epoll),
      socket_(std::make_unique<Socket>(fd)),
      channel_(std::make_unique<Channel>(epoll, fd)),
      address_(std::move(address)),
      read_buffer_(std::make_unique<Buffer>()),
      write_buffer_(std::make_unique<Buffer>()),
      state_(CONNECT) {
  socket_->nonblock();
  channel_->enable_read();
  channel_->useET();
  channel_->read_event_callback(
      [this]() { this->handle_onconnect_functioon_(this); });
  channel_->write_event_callback([this]() { this->write(); });
  channel_->disconnect_callback(
      [this]() { this->handle_disconnect_function_(); });
}

const std::vector<char>& Connection::read() {
  read_buffer_->clear();
  std::vector<char> buffer(DATA_MAX_SIZE);
  while (true) {
    memset(buffer.data(), 0, buffer.size());
    ssize_t read_bytes = ::read(socket_->fd(), buffer.data(), buffer.size());
    if (read_bytes > 0) {
      // 读取到数据，追加到读缓冲区
      read_buffer_->append(buffer, read_bytes);
    } else if (read_bytes == 0) {
      // 客户端断开连接
      state_ = DISCONNECT;
      break;
    } else if (read_bytes == -1 && errno == EINTR) {
      // 信号中断，继续读取
      continue;
    } else if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      // 非阻塞I/O下没有数据可读
      break;
    } else if (read_bytes == -1) {
      // 发生错误，做断开连接处理
      Log::error("Connection {} read error: {}", socket_->fd(),
                 strerror(errno));
      state_ = DISCONNECT;
      break;
    } else {
      // 未知异常，做断开连接处理
      Log::error("Connection {} read unknown error: {}", socket_->fd(),
                 strerror(errno));
      state_ = DISCONNECT;
      break;
    }
  }
  return read_buffer_->buffer();
}

void Connection::write(std::vector<char> data) {
  this->append(data);
  write();
}

void Connection::write() {
  int data_bytes = write_buffer_->size();
  if (data_bytes == 0) {
    return;
  }

  const char* data = write_buffer_->data();

  size_t left_bytes = data_bytes;
  ssize_t write_bytes = 0;

  while (left_bytes > 0) {
    write_bytes =
        ::write(socket_->fd(), data + (data_bytes - left_bytes), left_bytes);
    if (write_bytes > 0) {
      // 发送出数据
      left_bytes -= write_bytes;
    } else if (write_bytes == 0) {
      // 客户端断开连接
      state_ = DISCONNECT;
      break;
    } else if (write_bytes == -1 && errno == EINTR) {
      // 信号中断，继续写
      continue;
    } else if (write_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      // 非阻塞I/O发送缓冲区已满，等待下次发送，边缘触发模式下缓冲区从满到不满会触发写事件
      write_buffer_->erase(data_bytes - left_bytes);  // 清除已经写的数据
      channel_->enable_write();  // 开启监视写事件，等待下次写
      break;
    } else if (write_bytes == -1) {
      // 发送错误，断开连接
      Log::error("Connection {} write error: {}", socket_->fd(),
                 strerror(errno));
      state_ = DISCONNECT;
      break;
    } else {
      // 未知错误
      Log::error("Connection {} write unknown error: {}", socket_->fd(),
                 strerror(errno));
      state_ = DISCONNECT;
      break;
    }
  }

  if (left_bytes == 0) {
    write_buffer_->erase(data_bytes);
    channel_->disable_write();
  }
}

const std::vector<char> Connection::read_buffer() const {
  return read_buffer_->buffer();
}

void Connection::append(std::vector<char> data) {
  write_buffer_->append(data, data.size());
}

Connection::STATE Connection::state() { return state_; }

void Connection::handle_onconnect_function(
    std::function<void(Connection*)> function) {
  handle_onconnect_functioon_ = function;
}

void Connection::handle_disconnect_function(std::function<void()> function) {
  handle_disconnect_function_ = function;
}
