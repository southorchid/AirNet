#pragma once

#include <sys/types.h>

#include <string>
#include <vector>

class Buffer {
 public:
  Buffer();
  void clear();

  void append(const std::vector<char> &data, ssize_t len);
  void append(const std::string &data);

  size_t size() const;

  const char *data() const;

  const std::vector<char> &buffer() const;

  const std::string str() const;

  void erase(int len);

  bool empty();

 private:
  std::vector<char> data_;
};