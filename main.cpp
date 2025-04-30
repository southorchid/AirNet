#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <memory>

#include "Acceptor.h"
#include "Channel.h"
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"

Epoll epoll;

void newconnect(int fd, std::unique_ptr<InetAddress> address);

void onconnect(int fd);

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
        std::cout << "Client " << fd << " error" << std::endl;
        epoll.del(fd);
        close(fd);
      }
      if (reevents & EPOLLHUP) {
        std::cout << "Client " << fd << " disconnected" << std::endl;
        epoll.del(fd);
        close(fd);
      }
      if (reevents & EPOLLPRI) {
      }
      if (reevents & EPOLLIN) {
        ch->handle_read_event();
      }
      if (reevents & EPOLLOUT) {
      }
      if (reevents & EPOLLRDHUP) {
      }
    }
  }

  return 0;
}

void newconnect(int fd, std::unique_ptr<InetAddress> address) {
  Socket *client_sock = new Socket(fd);
  Channel *client_chan = new Channel(&epoll, fd);
  client_chan->handle_read_event_function([fd]() { onconnect(fd); });
  client_chan->enable_read();
  Log::info("Client {} connected from {}:{}", fd, address->host(),
            address->port());
}

void onconnect(int fd) {
  char read_buffer[BUFFER_MAX_SIZE];
  memset(read_buffer, 0, sizeof(read_buffer));
  int read_bytes = read(fd, read_buffer, BUFFER_MAX_SIZE);
  if (read_bytes <= 0) {
    std::cout << "Client " << fd << " disconected" << std::endl;
    epoll.del(fd);
    close(fd);
  } else {
    std::cout << "Received:\n" << read_buffer << std::endl;
    std::string write_buffer =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 2\r\n"
        "\r\n"
        "OK";
    int write_bytes = write(fd, write_buffer.c_str(), write_buffer.size());
    if (write_bytes == -1) {
      perror("Socket write failed");
      epoll.del(fd);
      close(fd);
    } else {
      std::cout << "Send:\n" << write_buffer << std::endl;
    }
  }
}
