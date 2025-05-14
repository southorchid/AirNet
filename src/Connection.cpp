#include "Connection.h"

Connection::Connection(std::shared_ptr<EventLoop> loop, int fd,
                       std::unique_ptr<InetAddress> address)
    : loop_(loop),
      socket_(std::make_unique<Socket>(fd)),
      channel_(std::make_unique<Channel>(loop, fd)),
      address_(std::move(address)),
      read_buffer_(std::make_unique<Buffer>()),
      write_buffer_(std::make_unique<Buffer>()),
      state_(CONNECT) {
  socket_->nonblock();
  channel_->enable_read();
  channel_->useET();
  channel_->read_event_callback([this]() {
    if (state_ == READCLOSE || state_ == DISCONNECT) {
      return;
    }
    this->onconect_callback_(shared_from_this());
  });
  channel_->write_event_callback([this]() {
    if (state_ == WRITECLOSE || state_ == DISCONNECT) {
      return;
    }
    this->write();
  });
  channel_->disconnect_callback([this]() { this->disconnect_callback_(); });

  channel_->close_read_callback([this]() { this->close_read(); });
  channel_->close_write_callback([this]() { this->close_write(); });
}

void Connection::read() {
  read_buffer_->clear();
  std::vector<char> buffer(DATA_MAX_SIZE);
  while (true) {
    memset(buffer.data(), 0, buffer.size());
    ssize_t read_bytes = ::read(socket_->fd(), buffer.data(), buffer.size());
    if (read_bytes > 0) {
      // 读取到数据，追加到读缓冲区
      read_buffer_->append(buffer, read_bytes);
    } else if (read_bytes == 0) {
      // 客户端断开写端
      Log::info("Connection {} read 0 bytes, close read!", socket_->fd());
      channel_->close_read();
      break;
    } else if (read_bytes == -1 && errno == EINTR) {
      // 信号中断，继续读取
      continue;
    } else if (read_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      // 非阻塞I/O下没有数据可读
      break;
    } else if (read_bytes == -1) {
      // 发生错误，断开连接
      Log::error("Connection {} read error: {}", socket_->fd(),
                 strerror(errno));
      state_ = DISCONNECT;
      channel_->close();
      break;
    } else {
      // 未知异常，断开连接
      Log::error("Connection {} read unknown error: {}", socket_->fd(),
                 strerror(errno));
      state_ = DISCONNECT;
      channel_->close();
      break;
    }
  }
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
      channel_->close_write();
      break;
    } else if (write_bytes == -1 && errno == EINTR) {
      // 信号中断，继续写
      continue;
    } else if (write_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      // 非阻塞I/O发送缓冲区已满，等待下次发送，边缘触发模式下缓冲区从满到不满会触发写事件
      write_buffer_->erase(data_bytes - left_bytes);  // 清除已经写的数据
      channel_->enable_write();  // 开启监视写事件，等待下次写
      break;
    } else if (write_bytes == -1 && errno == EPIPE) {
      channel_->close_write();
      break;
    } else if (write_bytes == -1) {
      // 发送错误，断开连接
      Log::error("Connection {} write error: {}", socket_->fd(),
                 strerror(errno));
      state_ = DISCONNECT;
      channel_->close();
      break;
    } else {
      // 未知错误
      Log::error("Connection {} write unknown error: {}", socket_->fd(),
                 strerror(errno));
      state_ = DISCONNECT;
      channel_->close();
      break;
    }
  }

  if (left_bytes == 0) {
    write_buffer_->erase(data_bytes);
    channel_->disable_write();
  }
}

void Connection::close_read() {
  // 读端已经关闭或连接已断开则不需要关闭读端
  if (state_ != READCLOSE && state_ != DISCONNECT) {
    if (shutdown(socket_->fd(), SHUT_RD) == -1) {
      Log::warn("Connection {} shutdown read failed: {}", socket_->fd(),
                strerror(errno));
      return;
    }
    channel_->disable_read();
  }
  if (state_ == CONNECT) {
    // 原来的状态是正常连接则将状态更新为读端关闭
    state_ = READCLOSE;
  } else if (state_ == WRITECLOSE) {
    // 原来的状态是写端关闭则将状态更新为连接断开
    state_ = DISCONNECT;
    channel_->close();
  }
}

void Connection::close_write() {
  if (state_ == WRITECLOSE || state_ == DISCONNECT) {
    return;
  }
  if (shutdown(socket_->fd(), SHUT_WR) == -1) {
    Log::warn("Connection {} shutdown write failed: {}", socket_->fd(),
              strerror(errno));
  }
  channel_->disable_write();
  if (state_ == CONNECT) {
    state_ = WRITECLOSE;
  } else if (state_ == READCLOSE) {
    state_ = DISCONNECT;
    channel_->close();
  }
}

const Buffer& Connection::read_buffer() const { return *read_buffer_; }

Connection::STATE Connection::state() { return state_; }

void Connection::onconect_callback(
    std::function<void(std::shared_ptr<Connection>)> func) {
  onconect_callback_ = func;
}

void Connection::disconnect_callback(std::function<void()> function) {
  disconnect_callback_ = function;
}
