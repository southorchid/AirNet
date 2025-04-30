#include "Log.h"

void Log::format(std::ostringstream& oss, const std::string& format) {
  oss << format;
}

const std::string Log::level_[COUNT] = {"DEBUG", "INFO", "WARN", "ERROR",
                                        "FATAL"};