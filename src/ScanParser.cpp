#include <string>
#include <regex>
#include "ScanParser.hpp"

using namespace std;

string ScanParser::trim(string s) {
  size_t first = s.find_first_not_of(' ');
  size_t last = s.find_last_not_of(' ');
  return s.substr(first, (last - first + 1));
}

string ScanParser::getOp(string v) {
  v = ScanParser::trim(v);
  regex r("^(=|>(?=[^=])|<(?=[^=])|>=|<=|!)");
  auto begin = sregex_iterator(v.begin(), v.end(), r);
  auto end = sregex_iterator();

  string matched;
  for (sregex_iterator i = begin; i != end; ++i) {
    smatch m = *i;
    matched = m.str();
  }
  return matched;
}

ScanParser::OpType ScanParser::stringToOpType(string s) {
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
  return ScanParser::Eq;
}

string ScanParser::getValue(string v) {
  v = ScanParser::trim(v);
  regex r("^(=|>(?=[^=])|<(?=[^=])|>=|<=|!)");
  return ScanParser::trim(regex_replace(v, r, ""));
}

bool ScanParser::isArray(string v) {
  v = ScanParser::trim(v);
  if (v.find(",") != string::npos)
    return true;
  return false;
}

ScanParser::OpType ScanParser::getOpType(string v) {
  return stringToOpType(getOp(v));
}
