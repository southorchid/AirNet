#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "Acceptor.h"
#include "Channel.h"
#include "Connection.h"
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"

Epoll epoll;

std::unordered_map<int, std::unique_ptr<Connection>> connections;

void newconnect(int fd, std::unique_ptr<InetAddress> address);

void disconnect(int fd);

void onconnect(Connection *conn);

const int BUFFER_MAX_SIZE = 1024;

int main() {
  Acceptor acceptor(&epoll);
  acceptor.bind("0.0.0.0", 8080);
  acceptor.listen();
  acceptor.handle_newconnect_function(newconnect);

  while (true) {
    auto channels = epoll.wait();
    for (auto ch : channels) {
      int fd = ch->fd();
      uint32_t reevents = ch->reevents();
      if (reevents & EPOLLERR) {
        ch->handle_disconnect();
        break;
      }
      if (reevents & EPOLLHUP) {
        ch->handle_disconnect();
        break;
      }
      if (reevents & EPOLLPRI) {
        ch->handle_read_event();
      }
      if (reevents & EPOLLIN) {
        ch->handle_read_event();
      }
      if (reevents & EPOLLOUT) {
        ch->handle_write_event();
      }
      if (reevents & EPOLLRDHUP) {
        ch->handle_disconnect();
        break;
      }
    }
  }

  return 0;
}

void newconnect(int fd, std::unique_ptr<InetAddress> address) {
  Log::info("Client {} connected from {}:{}", fd, address->host(),
            address->port());
  std::unique_ptr<Connection> conn =
      std::make_unique<Connection>(&epoll, fd, std::move(address));
  conn->handle_onconnect_function(std::bind(onconnect, std::placeholders::_1));
  conn->handle_disconnect_function([fd]() { disconnect(fd); });
  connections.insert(std::make_pair(fd, std::move(conn)));
}

void disconnect(int fd) {
  auto it = connections.find(fd);
  if (it == connections.end()) {
    return;
  }
  epoll.del(fd);
  connections.erase(fd);
  Log::info("Client {} disconnected", fd);
}

void onconnect(Connection *conn) {
  std::cout << "Recevied:\n" << conn->read().data() << std::endl;
  std::string write_buffer =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 2\r\n"
      "\r\n"
      "OK";
  conn->write(std::vector<char>(write_buffer.begin(), write_buffer.end()));
}
