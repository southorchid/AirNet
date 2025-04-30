#include "InetAddress.h"

InetAddress::InetAddress() { memset(&address_, 0, sizeof(address_)); }

InetAddress::InetAddress(const std::string& host, int port) {
  memset(&address_, 0, sizeof(address_));
  address_.sin_family = AF_INET;
  address_.sin_addr.s_addr = inet_addr(host.c_str());
  address_.sin_port = htons(port);
}

sockaddr* InetAddress::addr() const { return (struct sockaddr*)&address_; }

socklen_t InetAddress::len() const { return sizeof(address_); }

std::string InetAddress::host() const { return inet_ntoa(address_.sin_addr); }

int InetAddress::port() const { return ntohs(address_.sin_port); }
