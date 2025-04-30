#pragma once

#include <iostream>
#include <sstream>

class Log {
 public:
  enum LEVEL { DEBUG = 0, INFO, WARN, ERROR, FATAL, COUNT };

  template <class... Args>
  static void debug(const std::string &format, Args &&...args);

  template <class... Args>
  static void info(const std::string &format, Args &&...args);

  template <class... Args>
  static void warn(const std::string &format, Args &&...args);

  template <class... Args>
  static void error(const std::string &format, Args &&...args);

  template <class... Args>
  static void fatal(const std::string &format, Args &&...args);

  static void format(std::ostringstream &oss, const std::string &format);

  template <class T, class... Args>
  static void format(std::ostringstream &oss, const std::string &format,
                     T &&arg, Args &&...args);

  template <class... Args>
  static void output(LEVEL level, const std::string &file, int line,
                     const std::string &format, Args &&...args);

  static const std::string level_[COUNT];
};

template <class... Args>
inline void Log::debug(const std::string &format, Args &&...args) {
  output(DEBUG, __FILE__, __LINE__, format, std::forward<Args>(args)...);
}

template <class... Args>
inline void Log::info(const std::string &format, Args &&...args) {
  output(INFO, __FILE__, __LINE__, format, std::forward<Args>(args)...);
}

template <class... Args>
inline void Log::warn(const std::string &format, Args &&...args) {
  output(WARN, __FILE__, __LINE__, format, std::forward<Args>(args)...);
}

template <class... Args>
inline void Log::error(const std::string &format, Args &&...args) {
  output(ERROR, __FILE__, __LINE__, format, std::forward<Args>(args)...);
}

template <class... Args>
inline void Log::fatal(const std::string &format, Args &&...args) {
  output(FATAL, __FILE__, __LINE__, format, std::forward<Args>(args)...);
}

template <class T, class... Args>
inline void Log::format(std::ostringstream &oss, const std::string &format,
                        T &&arg, Args &&...args) {
  auto pos = format.find("{}");
  if (pos != std::string::npos) {
    oss << format.substr(0, pos);
    oss << std::forward<T>(arg);
    Log::format(oss, format.substr(pos + 2), std::forward<Args>(args)...);
  } else {
    oss << format;
  }
}

template <class... Args>
inline void Log::output(LEVEL level, const std::string &file, int line,
                        const std::string &format, Args &&...args) {
  std::ostringstream oss;
  oss << "[" << level_[level] << "]" << file << ":" << line << "-";
  Log::format(oss, format, std::forward<Args>(args)...);
  std::cout << oss.str() << std::endl;
}
