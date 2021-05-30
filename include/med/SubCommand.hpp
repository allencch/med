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
    Int8,
    Int16,
    Int32,
    Int64,
    Float32,
    Float64,
    Wildcard
  };
  static constexpr const char* CMD_REGEX = "^(s|w|i8|i16|i32|i64|f32|f64):";
  static constexpr const char* CMD_STRING = "'(.+?)'";

  explicit SubCommand(const string &s);
  Operands getOperands();
  int getWildcardSteps();
  Command getCmd();
  size_t getSize();
  ScanParser::OpType op;

  /**
   * @return tuple of boolean match result, and int of steps involved
   */
  tuple<bool, int> match(Byte* address);

  static Command parseCmd(const string &s);
  static string getCmdString(const string &s);
private:
  string stripCommand(const string &s);

  Operands operands;
  Command cmd;
  int wildcardSteps;
};

#endif
