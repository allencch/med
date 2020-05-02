#include "med/ScanCommand.hpp"

ScanCommand::ScanCommand() {}

vector<SubCommand> ScanCommand::getSubCommands() {
  return subCommands;
}
