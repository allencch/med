#include <cstring>
#include <iostream>
#include <vector>
#include <unicode/unistr.h>

#include "med/Coder.hpp"

using namespace std;

// Based on http://faithandbrave.hateblo.jp/entry/20100318/1268896477
string convertBig5ToUtf8(const string& input) {
  return convertCode(input, "big5", "utf8");
}

string convertCode(const string& input, const char* from, const char* to) {
  icu::UnicodeString src(input.c_str(), from);
  int length = src.extract(0, src.length(), NULL, to);
  vector<char> result(length + 1);
  src.extract(0, src.length(), &result[0], to);
  return string(result.begin(), result.end() - 1);
}

string convertToUtf8(const string& input, const char* from) {
  return convertCode(input, from, "utf8");
}

string convertFromUtf8(const string& input, const char* to) {
  return convertCode(input, "utf8", to);
}
