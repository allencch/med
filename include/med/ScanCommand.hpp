#ifndef SCAN_COMMAND_HPP
#define SCAN_COMMAND_HPP

#include <vector>
#include <string>
#include "med/SubCommand.hpp"

using namespace std;

class ScanCommand {
public:
  explicit ScanCommand(const string& s);
  vector<SubCommand> getSubCommands();
private:
  vector<SubCommand> subCommands;
};
#endif
