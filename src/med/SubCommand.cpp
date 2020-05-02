#include <regex>

#include "med/SubCommand.hpp"
#include "mem/StringUtil.hpp"

SubCommand::SubCommand() {}

SubCommand::SubCommand(const string &s) {
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

SubCommand::Command SubCommand::getCmd(const string& s) {
  auto cmd = getCmdString(s);
  if (cmd == "s:") {
    return Command::Str;
  } else if (cmd == "w:") {
    return Command::Wildcard;
  }
  return Command::Noop;
}
