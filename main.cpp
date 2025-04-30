#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "Channel.h"
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"

const int BUFFER_MAX_SIZE = 1024;

int main() {
  Socket server_sock;

  InetAddress server_addr("0.0.0.0", 8080);

  if (bind(server_sock.fd(), server_addr.addr(), server_addr.len()) == -1) {
    perror("Socket bind failed");
    throw std::runtime_error("Socket bind failed");
  }

  if (listen(server_sock.fd(), SOMAXCONN) == -1) {
    perror("Socket listen failed");
    throw std::runtime_error("Socket listen failed");
  }

  Epoll epoll;

  Channel *server_chan = new Channel(&epoll, server_sock.fd());
  server_chan->enable_read();

  while (true) {
    auto channels = epoll.wait();
    for (auto ch : channels) {
      int fd = ch->fd();
      uint32_t reevents = ch->reevents();
      if (reevents & EPOLLERR) {
        std::cout << "Client " << fd << " error" << std::endl;
        epoll.del(ch);
        close(fd);
      }
      if (reevents & EPOLLHUP) {
        std::cout << "Client " << fd << " disconnected" << std::endl;
        epoll.del(ch);
        close(fd);
      }
      if (reevents & EPOLLPRI) {
      }
      if (reevents & EPOLLIN) {
        if (fd == server_chan->fd()) {
          InetAddress client_addr;
          socklen_t client_addr_len = client_addr.len();
          int clientfd =
              accept(server_sock.fd(), client_addr.addr(), &client_addr_len);
          Channel *client_chan = new Channel(&epoll, clientfd);
          client_chan->enable_read();
          if (clientfd == -1) {
            perror("Socket accept failed");
            throw std::runtime_error("Socket accept failed");
          } else {
            std::cout << "Client " << clientfd << " connected from "
                      << client_addr.host() << ":" << client_addr.port()
                      << std::endl;
          }
        } else {
          char read_buffer[BUFFER_MAX_SIZE];
          memset(read_buffer, 0, sizeof(read_buffer));
          int read_bytes = read(fd, read_buffer, BUFFER_MAX_SIZE);
          if (read_bytes <= 0) {
            std::cout << "Client " << fd << " disconected" << std::endl;
            epoll.del(ch);
            close(fd);
          } else {
            std::cout << "Received:\n" << read_buffer << std::endl;
            std::string write_buffer =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 2\r\n"
                "\r\n"
                "OK";
            int write_bytes =
                write(fd, write_buffer.c_str(), write_buffer.size());
            if (write_bytes == -1) {
              perror("Socket write failed");
              epoll.del(ch);
              close(fd);
            } else {
              std::cout << "Send:\n" << write_buffer << std::endl;
            }
          }
        }
        if (reevents & EPOLLOUT) {
        }
        if (reevents & EPOLLRDHUP) {
        }
      }
    }
  }

  return 0;
}