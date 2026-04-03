#include <cstring>
#include <vector>
#include <unicode/unistr.h>

#include "med/Coder.hpp"

using namespace std;

string convertBig5ToUtf8(const string& input) {
  return convertCode(input, "big5", "utf8");
}

string convertCode(const string& input, const char* from, const char* to) {
  icu::UnicodeString src(input.c_str(), from);
  int length = src.extract(0, src.length(), NULL, to);
  if (length < 0) return "";
  vector<char> result(length + 1);
  src.extract(0, src.length(), &result[0], to);
  return string(result.begin(), result.begin() + length);
}

string convertToUtf8(const string& input, const char* from) {
  return convertCode(input, from, "utf8");
}

string convertFromUtf8(const string& input, const char* to) {
  return convertCode(input, "utf8", to);
}
