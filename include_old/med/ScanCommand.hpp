#ifndef SCAN_COMMAND_HPP
#define SCAN_COMMAND_HPP

#include <vector>
#include <string>
#include "med/MedTypes.hpp"
#include "med/SubCommand.hpp"

using namespace std;

class ScanCommand {
public:
  explicit ScanCommand(const string& s, const string& scanType = SCAN_TYPE_CUSTOM);
  vector<SubCommand> getSubCommands();
  size_t getSize();

  bool match(Byte* address);
  string getFirstScanType();
private:
  vector<SubCommand> subCommands;

  size_t _getSize(); // Memoization
  size_t size;
  string commandString;
  string scanType;
};
#endif
