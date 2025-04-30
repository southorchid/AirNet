#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#include <string>

class InetAddress {
 public:
  InetAddress();
  InetAddress(const std::string& host, int port);

  sockaddr* addr() const;

  socklen_t len() const;

  std::string host() const;

  int port() const;

 private:
  struct sockaddr_in address_;
};