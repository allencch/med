#include <regex>

#include "med/SubCommand.hpp"
#include "med/ScanParser.hpp"
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
  regex r(":(\\d+)");
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

SubCommand::SubCommand(const string &s) {
  cmd = parseCmd(s);
  wildcardSteps = 0;
  if (cmd == Command::Noop) {
    // NOTE: Default to int32 first
    operands = ScanParser::valueToOperands(s, SCAN_TYPE_INT_32);
  } else if (cmd == Command::Str) {
    string valueStr = extractString(s);
    operands = ScanParser::valueToOperands(valueStr, SCAN_TYPE_STRING);
  } else if (cmd == Command::Wildcard) {
    wildcardSteps = extractNumber(s);
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

int SubCommand::getWildcardSteps() {
  return wildcardSteps;
}
