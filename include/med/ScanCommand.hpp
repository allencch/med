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
  size_t getSize();
private:
  vector<SubCommand> subCommands;

  size_t _getSize(); // Memoization
  size_t size;
};
#endif
