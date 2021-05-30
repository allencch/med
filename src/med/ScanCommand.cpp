#include <tuple>
#include "med/ScanCommand.hpp"
#include "med/ScanParser.hpp"

ScanCommand::ScanCommand(const string& s) {
  commandString = s;
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

bool ScanCommand::match(Byte* address) {
  bool result = true;
  Byte *ptr = address;
  for (size_t i = 0; i < subCommands.size(); i++) {
    SubCommand subCmd = subCommands[i];
    tuple<bool, int> subResult = subCmd.match(ptr);
    bool match = std::get<0>(subResult);
    int step = std::get<1>(subResult);

    if (!match) return false;

    ptr += step;
  }
  return result;
}

string ScanCommand::getFirstScanType() {
  return SubCommand::getScanType(commandString);
}
