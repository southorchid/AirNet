#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

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
  epoll.add(server_sock.fd());

  while (true) {
    auto fds = epoll.wait();
    for (auto fd : fds) {
      if (fd == server_sock.fd()) {
        InetAddress client_addr;
        socklen_t client_addr_len = client_addr.len();
        int clientfd =
            accept(server_sock.fd(), client_addr.addr(), &client_addr_len);
        Socket client_sock(clientfd);
        if (client_sock.fd() == -1) {
          perror("Socket accept failed");
          throw std::runtime_error("Socket accept failed");
        } else {
          epoll.add(clientfd);
          std::cout << "Client " << client_sock.fd() << " connected from "
                    << client_addr.host() << ":" << client_addr.port()
                    << std::endl;
        }
      } else {
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
          int write_bytes =
              write(fd, write_buffer.c_str(), write_buffer.size());
          if (write_bytes == -1) {
            perror("Socket write failed");
          } else {
            std::cout << "Send:\n" << write_buffer << std::endl;
          }
        }
      }
    }
  }

  return 0;
}