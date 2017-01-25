#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include "ScanParser.hpp"

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

