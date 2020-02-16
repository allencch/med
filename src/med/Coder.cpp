#include <cstring>
#include <iostream>
#include <vector>
#include <unicode/unistr.h>
#include <unicode/ucnv.h>
#include <unicode/ustring.h>

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

string _convertCodeUsingC(const string& input, const char* from, const char* to) {
  const char* inputPtr = input.c_str();
  const int bufLength = 255;

  UErrorCode status = U_ZERO_ERROR;
  UConverter* conv = ucnv_open(from, &status);
  if (U_FAILURE(status)) {
    fprintf(stderr, "Open ucnv error: %s\n", u_errorName(status));
    return "";
  }

  UChar buffer[bufLength];
  ucnv_toUChars(conv, buffer, bufLength, inputPtr, input.size(), &status);
  if (U_FAILURE(status)) {
    fprintf(stderr, "toUChars error: %s\n", u_errorName(status));
    return "";
  }

  conv = ucnv_open(to, &status);
  if (U_FAILURE(status)) {
    fprintf(stderr, "Open ucnv error: %s\n", u_errorName(status));
    return "";
  }

  char output[bufLength];
  ucnv_fromUChars(conv, output, bufLength, buffer, u_strlen(buffer), &status);
  if (U_FAILURE(status)) {
    fprintf(stderr, "fromUChars error: %s\n", u_errorName(status));
    return "";
  }

  return string(output);
}

string convertToUtf8(const string& input, const char* from) {
  return convertCode(input, from, "utf8");
}

string convertFromUtf8(const string& input, const char* to) {
  return convertCode(input, "utf8", to);
}
