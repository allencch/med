#ifndef SUB_COMMAND_HPP
#define SUB_COMMAND_HPP

#include <string>
#include "med/Operands.hpp"
using namespace std;

class SubCommand {
public:
  enum Command {
    Noop,
    Str,
    Wildcard
  };
  static constexpr const char* CMD_REGEX = "^(s|w):";
  static constexpr const char* CMD_STRING = "'(.+?)'";

  explicit SubCommand(const string &s);
  Operands getOperands();
  int getWildcardSteps();
  Command getCmd();

  static Command parseCmd(const string &s);
private:
  Operands operands;
  Command cmd;
  int wildcardSteps;
};

#endif
