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
  SubCommand();
  explicit SubCommand(const string &s);
  Operands getOperands();
  Command getCmd();

  static Command parseCmd(const string &s);
private:
  Operands operands;
  Command cmd;
};

#endif
