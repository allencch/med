#ifndef SCAN_COMMAND_HPP
#define SCAN_COMMAND_HPP

#include <vector>
#include "med/SubCommand.hpp"

using namespace std;

class ScanCommand {
public:
  ScanCommand();
  vector<SubCommand> getSubCommands();
private:
  vector<SubCommand> subCommands;
};
#endif
