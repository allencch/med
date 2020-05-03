#include "med/ScanCommand.hpp"
#include "med/ScanParser.hpp"

ScanCommand::ScanCommand(const string& s) {
  auto values = ScanParser::getValues(s);

  for (size_t i = 0; i < values.size(); i++) {
    auto value = values[i];
    SubCommand subCmd(value);
    subCommands.push_back(subCmd);
  }

  size = _getSize();
}

vector<SubCommand> ScanCommand::getSubCommands() {
  return subCommands;
}

size_t ScanCommand::_getSize() {
  size_t size = 0;
  for (size_t i = 0; i < subCommands.size(); i++) {
    auto subCommand = subCommands[i];
    size += subCommand.getSize();
  }
  return size;
}

size_t ScanCommand::getSize() {
  return size;
}
