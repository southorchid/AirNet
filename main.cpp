#include "Server.h"

void onconnect(std::shared_ptr<Connection> conn);

int main() {
  Server server("0.0.0.0", 8080, Driver::SELECT, 1, onconnect);
  server.run();
  return 0;
}

void onconnect(std::shared_ptr<Connection> conn) {
  conn->read();
  std::cout << "Recevied:\n" << conn->read_buffer().str() << std::endl;
  std::string write_buffer =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 2\r\n"
      "\r\n"
      "OK";
  conn->write(write_buffer);
}
