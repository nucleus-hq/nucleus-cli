#ifndef PROMPT_HPP
#define PROMPT_HPP

#include <iostream>
#include <sstream>
#include <string>

namespace prompt {

// internal formatter
template <typename... Args>
std::string format(const std::string &msg, Args... args) {
  std::ostringstream oss;
  oss << msg;
  ((oss << " " << args), ...);
  return oss.str();
}

template <typename... Args> void Stage(const std::string &msg, Args... args) {
  std::cout << "[*] " << format(msg, args...) << std::endl;
}

template <typename... Args> void Action(const std::string &msg, Args... args) {
  std::cout << "[+] " << format(msg, args...) << std::endl;
}

template <typename... Args> void Success(const std::string &msg, Args... args) {
  std::cout << "[✓] " << format(msg, args...) << std::endl;
}

template <typename... Args> void Warn(const std::string &msg, Args... args) {
  std::cout << "[!] " << format(msg, args...) << std::endl;
}

template <typename... Args>
std::string Fail(const std::string &msg, Args... args) {
  std::cout << "[x] " << format(msg, args...) << std::endl;
  return format(msg, args...);
}

template <typename... Args>
std::string Ask(const std::string &msg, Args... args) {
  std::cout << "[?] " << format(msg, args...) << " ";
  std::string input;
  std::getline(std::cin, input);
  return input;
}

} // namespace prompt

#endif
