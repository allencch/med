#include <iostream>
#include <regex>

#include "med/SubCommand.hpp"
#include "med/ScanParser.hpp"
#include "med/MemOperator.hpp"
#include "mem/StringUtil.hpp"

string extractString(const string& s) {
  string value = StringUtil::trim(s);
  regex r(SubCommand::CMD_STRING);
  smatch match;
  if (std::regex_search(value, match, r)) {
    return match[1];
  }
  return "";
}

int extractNumber(const string &s) {
  string value = StringUtil::trim(s);
  regex r(":\\s*?(\\d+)");
  smatch match;
  string matched;
  if (std::regex_search(value, match, r)) {
    matched = match[1];
  }
  if (matched.size()) {
    return stoi(matched);
  }
  return 0;
}

SubCommand::SubCommand(const string &s, const string &scanType) {
  cmd = parseCmd(s);
  wildcardSteps = 0;
  string valueStr;

  string stripped = stripCommand(s);
  op = ScanParser::getOpType(stripped);

  switch (cmd) {
  case Command::Int8:
    operands = ScanParser::valueToOperands(stripped, SCAN_TYPE_INT_8, op);
    break;
  case Command::Int16:
    operands = ScanParser::valueToOperands(stripped, SCAN_TYPE_INT_16, op);
    break;
  case Command::Int32:
    operands = ScanParser::valueToOperands(stripped, SCAN_TYPE_INT_32, op);
    break;
  case Command::Int64:
    operands = ScanParser::valueToOperands(stripped, SCAN_TYPE_INT_64, op);
    break;
  case Command::Float32:
    operands = ScanParser::valueToOperands(stripped, SCAN_TYPE_FLOAT_32, op);
    break;
  case Command::Float64:
    operands = ScanParser::valueToOperands(stripped, SCAN_TYPE_FLOAT_64, op);
    break;
  case Command::Str:
    valueStr = extractString(s);
    operands = ScanParser::valueToOperands(valueStr, SCAN_TYPE_STRING, op);
    break;
  case Command::Wildcard:
    wildcardSteps = extractNumber(s);
    break;
  case Command::Noop:
    string fallbackType = scanType == SCAN_TYPE_CUSTOM ? SCAN_TYPE_INT_32 : scanType;
    operands = ScanParser::valueToOperands(stripped, fallbackType, op);
    break;
  }
}

string SubCommand::getScanType(const string &s, const string &scanType) {
  auto cmd = parseCmd(s);
  switch (cmd) {
  case Command::Wildcard:
  case Command::Int8:
    return SCAN_TYPE_INT_8;
  case Command::Int16:
    return SCAN_TYPE_INT_16;
  case Command::Int32:
    return SCAN_TYPE_INT_32;
  case Command::Int64:
    return SCAN_TYPE_INT_64;
  case Command::Float32:
    return SCAN_TYPE_FLOAT_32;
  case Command::Float64:
    return SCAN_TYPE_FLOAT_64;
  case Command::Str:
    return SCAN_TYPE_STRING;
  case Command::Noop:
    if (scanType == SCAN_TYPE_CUSTOM) {
      return SCAN_TYPE_INT_32;
    }
    return scanType;
  }
  return SCAN_TYPE_INT_8;
}

Operands SubCommand::getOperands() {
  return operands;
}

string SubCommand::getCmdString(const string& s) {
  string value = StringUtil::trim(s);
  regex r(SubCommand::CMD_REGEX);
  auto begin = sregex_iterator(value.begin(), value.end(), r);
  auto end = sregex_iterator();

  string matched;
  for (sregex_iterator i = begin; i != end; ++i) {
    smatch m = *i;
    matched = m.str();
  }
  return matched;
}

string SubCommand::stripCommand(const string &s) {
  string copy = s;
  return StringUtil::replace(copy, getCmdString(s), "");
}

SubCommand::Command SubCommand::parseCmd(const string& s) {
  auto cmd = getCmdString(s);
  if (cmd == "s:") {
    return Command::Str;
  }
  else if (cmd == "i8:") {
    return Command::Int8;
  }
  else if (cmd == "i16:") {
    return Command::Int16;
  }
  else if (cmd == "i32:") {
    return Command::Int32;
  }
  else if (cmd == "i64:") {
    return Command::Int64;
  }
  else if (cmd == "f32:") {
    return Command::Float32;
  }
  else if (cmd == "f64:") {
    return Command::Float64;
  }
  else if (cmd == "w:") {
    return Command::Wildcard;
  }
  return Command::Noop;
}

SubCommand::Command SubCommand::getCmd() {
  return cmd;
}

int SubCommand::getWildcardSteps() {
  return wildcardSteps;
}

size_t SubCommand::getSize() {
  switch (cmd) {
  case Command::Wildcard:
    return wildcardSteps;
  default:
    return operands.getFirstSize();
  }
  return 0;
}

tuple<bool, int> SubCommand::match(Byte* address) {
  bool matchResult;
  size_t size = getSize();
  switch (cmd) {
  case Command::Wildcard:
    matchResult = true;
    break;
  default:
    matchResult = memCompare(address, size, operands, op);
  }
  return make_tuple(matchResult, size);
}
