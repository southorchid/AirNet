#include "Buffer.h"

Buffer::Buffer() : data_() {}

void Buffer::clear() { data_.clear(); }

void Buffer::append(const std::vector<char> &data, ssize_t len) {
  for (int i = 0; i < len; ++i) {
    data_.emplace_back(data[i]);
  }
}

void Buffer::append(const std::string &data) {
  for (const auto ch : data) {
    data_.emplace_back(ch);
  }
}

size_t Buffer::size() const { return data_.size(); }

const char *Buffer::data() const { return data_.data(); }

const std::vector<char> &Buffer::buffer() const { return data_; }

const std::string Buffer::str() const {
  return std::string(data_.begin(), data_.end());
}

void Buffer::erase(int len) {
  for (int i = 1; i <= len; ++i) {
    if (data_.size() >= i) {
      data_.erase(data_.begin());
    }
  }
}

bool Buffer::empty() { return data_.empty(); }
