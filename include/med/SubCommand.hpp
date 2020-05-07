#ifndef SUB_COMMAND_HPP
#define SUB_COMMAND_HPP

#include <string>
#include <tuple>
#include "med/MedTypes.hpp"
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
  size_t getSize();

  /**
   * @return tuple of boolean match result, and int of steps involved
   */
  tuple<bool, int> match(Byte* address);

  static Command parseCmd(const string &s);
private:
  Operands operands;
  Command cmd;
  int wildcardSteps;
};

#endif
