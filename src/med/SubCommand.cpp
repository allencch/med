#include <regex>

#include "med/SubCommand.hpp"
#include "med/ScanParser.hpp"
#include "mem/StringUtil.hpp"

SubCommand::SubCommand() {}

string extractString(const string& s) {
  string value = StringUtil::trim(s);
  regex r(SubCommand::CMD_STRING);
  smatch match;
  if (std::regex_search(value, match, r)) {
    return match[1];
  }
  return "";
}

SubCommand::SubCommand(const string &s) {
  cmd = parseCmd(s);
  if (cmd == Command::Noop) {
    // NOTE: Default to int32 first
    operands = ScanParser::valueToOperands(s, SCAN_TYPE_INT_32);
  } else if (cmd == Command::Str) {
    string valueStr = extractString(s);
    operands = ScanParser::valueToOperands(valueStr, SCAN_TYPE_STRING);
  }
}

Operands SubCommand::getOperands() {
  return operands;
}

string getCmdString(const string& s) {
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

SubCommand::Command SubCommand::parseCmd(const string& s) {
  auto cmd = getCmdString(s);
  if (cmd == "s:") {
    return Command::Str;
  } else if (cmd == "w:") {
    return Command::Wildcard;
  }
  return Command::Noop;
}

SubCommand::Command SubCommand::getCmd() {
  return cmd;
}
