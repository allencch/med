#ifndef CODER_HPP
#define CODER_HPP

#include <string>

std::string convertBig5ToUtf8(const std::string& input);
std::string convertCode(const std::string& input, const char* from, const char* to);
std::string convertToUtf8(const std::string& input, const char* from);
std::string convertFromUtf8(const std::string& input, const char* to);

#endif
