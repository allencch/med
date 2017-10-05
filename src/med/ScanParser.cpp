#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include <iostream>

#include "med/ScanParser.hpp"
#include "med/MedException.hpp"
#include "med/MedCommon.hpp"

using namespace std;

string ScanParser::trim(const string &s) {
  size_t first = s.find_first_not_of(' ');
  size_t last = s.find_last_not_of(' ');
  return s.substr(first, (last - first + 1));
}

string ScanParser::getOp(const string &v) {
  string value = ScanParser::trim(v);
  regex r("^(=|>(?=[^=])|<(?=[^=>])|>=|<=|!|<>)");
  auto begin = sregex_iterator(value.begin(), value.end(), r);
  auto end = sregex_iterator();

  string matched;
  for (sregex_iterator i = begin; i != end; ++i) {
    smatch m = *i;
    matched = m.str();
  }
  return matched;
}

ScanParser::OpType ScanParser::stringToOpType(const string &s) {
  if (s == "<")
    return ScanParser::Lt;
  else if (s == ">")
    return ScanParser::Gt;
  else if (s == "<=")
    return ScanParser::Le;
  else if (s == ">=")
    return ScanParser::Ge;
  else if (s == "!")
    return ScanParser::Neq;
  else if (s == "<>")
    return ScanParser::Within;
  return ScanParser::Eq;
}

string ScanParser::getValue(const string &v) {
  string value = ScanParser::trim(v);
  regex r("^(=|>(?=[^=])|<(?=[^=>])|>=|<=|!|<>)");
  return ScanParser::trim(regex_replace(value, r, ""));
}

bool ScanParser::isArray(const string &v) {
  string value = ScanParser::trim(v);
  if (value.find(",") != string::npos)
    return true;
  return false;
}

ScanParser::OpType ScanParser::getOpType(const string &v) {
  return stringToOpType(getOp(v));
}

vector<string> ScanParser::split(const string &s, char delim) {
  stringstream ss(s);
  string token;
  vector<string> tokens;
  while (getline(ss, token, delim)) {
    tokens.push_back(ScanParser::trim(token));
  }
  return tokens;
}

vector<string> ScanParser::getValues(const string &v) {
  string value = ScanParser::getValue(v);
  vector<string> values = ScanParser::split(value, ',');
  return values;
}

bool ScanParser::isValid(const string &v) {
  if (ScanParser::getOpType(v) == ScanParser::Within &&
      !ScanParser::isArray(v))
    return false;
  return true;
}

Bytes ScanParser::getBytes(const string& v, const string& t) {
  if (t == SCAN_TYPE_STRING) {
    return ScanParser::getStringBytes(v);
  }
  else {
    return ScanParser::getNumericBytes(v, t);
  }
}

Bytes ScanParser::getNumericBytes(const string& v, const string& t) {
  vector<string> values = getValues(v);
  if (values.size() == 0) {
    throw MedException("Scan empty string");
  }

  int valueLength = scanTypeToSize(t);
  Byte* data = new Byte[valueLength * values.size()];

  Byte* pointer = data;
  for (int i = 0; i < (int)values.size(); i++) {
    stringToMemory(values[i], t, pointer);
    pointer += valueLength;
  }
  Bytes bytes(data, valueLength * values.size());
  return bytes;
}

Bytes ScanParser::getStringBytes(const string& v) {
  vector<string> values = getValues(v);
  if (values.size() == 0) {
    throw MedException("Scan empty string");
  }
  int valueLength = v.size();
  Byte* data = new Byte[valueLength + 1];

  char* pointer = (char*)data;
  for (int i = 0; i < (int)v.size(); i++, pointer++) {
    sprintf(pointer, "%c", v[i]);
  }

  Bytes bytes(data, valueLength);
  return bytes;
}
