#include "med/ScanCommand.hpp"
#include "med/ScanParser.hpp"

ScanCommand::ScanCommand(const string& s) {
  auto values = ScanParser::getValues(s);

  for (size_t i = 0; i < values.size(); i++) {
    auto value = values[i];
    SubCommand subCmd(value);
    subCommands.push_back(subCmd);
  }
}

vector<SubCommand> ScanCommand::getSubCommands() {
  return subCommands;
}
