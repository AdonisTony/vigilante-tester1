// Copyright (c) 2018-2021 Marco Wang <m.aesophor@gmail.com>. All rights reserved.
#ifndef VIGILANTE_LOGGER_H_
#define VIGILANTE_LOGGER_H_

#include <array>
#include <memory>
#include <string>

#include <cocos2d.h>

#define LOG_ERR vigilante::logger::Severity::ERROR
#define LOG_WARN vigilante::logger::Severity::WARNING
#define LOG_INFO vigilante::logger::Severity::INFO

// Example usage: VGLOG(LOG_INFO, __FILE__, "test msg %d", 5);
#define VGLOG(severity, format, ...)\
  cocos2d::log("[%s] [%s: %d] " format,\
      (vigilante::logger::_kSeverityStr[severity].c_str()),\
      (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__),\
      (__LINE__),\
      ##__VA_ARGS__)

namespace vigilante::logger {

enum Severity {
  ERROR,
  WARNING,
  INFO,
  SIZE
};

const std::array<std::string, Severity::SIZE> _kSeverityStr = {{
  "ERROR",
  "WARNING",
  "INFO"
}};

void segvHandler(int);

}  // namespace vigilante::logger

#endif // VIGILANTE_LOGGER_H_
