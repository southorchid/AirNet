#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

const int BUFFER_MAX_SIZE = 1024;

int main() {
  int serverfd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverfd == -1) {
    perror("Socket create failed");
    throw std::runtime_error("Socket create failed");
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(8080);

  if (bind(serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("Socket bind failed");
    throw std::runtime_error("Socket bind failed");
  }

  if (listen(serverfd, SOMAXCONN) == -1) {
    perror("Socket listen failed");
    throw std::runtime_error("Socket listen failed");
  }

  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  memset(&client_addr, 0, client_addr_len);

  int clientfd =
      accept(serverfd, (struct sockaddr *)&client_addr, &client_addr_len);
  if (clientfd == -1) {
    perror("Socket accept failed");
  } else {
    std::cout << "Client " << clientfd << " connected from "
              << inet_ntoa(client_addr.sin_addr) << ":"
              << htons(client_addr.sin_port) << std::endl;
  }

  char read_buffer[BUFFER_MAX_SIZE];
  memset(read_buffer, 0, sizeof(read_buffer));
  int read_bytes = read(clientfd, read_buffer, BUFFER_MAX_SIZE);
  if (read_bytes == -1) {
    perror("Socket read failed");
  } else {
    std::cout << "Received:\n" << read_buffer << std::endl;
  }

  std::string write_buffer =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 2\r\n"
      "\r\n"
      "OK";
  int write_bytes = write(clientfd, write_buffer.c_str(), write_buffer.size());
  if (write_bytes == -1) {
    perror("Socket write failed");
  } else {
    std::cout << "Send:\n" << write_buffer;
  }
  return 0;
}