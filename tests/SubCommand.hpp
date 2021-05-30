#include <string>
#include <cxxtest/TestSuite.h>

#include "med/SubCommand.hpp"

using namespace std;

class TestSubCommand : public CxxTest::TestSuite {
public:
  void test_getCmd() {
    string s = "1";
    SubCommand::Command cmd = SubCommand::parseCmd(s);
    TS_ASSERT_EQUALS(cmd, SubCommand::Noop);

    s = "s:'1'";
    cmd = SubCommand::parseCmd(s);
    TS_ASSERT_EQUALS(cmd, SubCommand::Str);

    s = "w:10";
    cmd = SubCommand::parseCmd(s);
    TS_ASSERT_EQUALS(cmd, SubCommand::Wildcard);

    s = "i8:5";
    cmd = SubCommand::parseCmd(s);
    TS_ASSERT_EQUALS(cmd, SubCommand::Int8);
  }

  void test_getOperands() {
    string s = "2";
    SubCommand subCmd(s);
    auto operands = subCmd.getOperands();
    auto operand = operands.getFirstOperand();
    auto bytes = operand.getBytes();
    TS_ASSERT_EQUALS(bytes[0], 2);
    TS_ASSERT_EQUALS(bytes[1], 0);
    TS_ASSERT_EQUALS(bytes[2], 0);
    TS_ASSERT_EQUALS(bytes[3], 0);
  }

  void test_getCmd_with_string() {
    string s = "s:'20'";
    SubCommand subCmd(s);
    auto operands = subCmd.getOperands();
    auto operand = operands.getFirstOperand();
    auto size = operands.getFirstSize();
    TS_ASSERT_EQUALS(size, 2);

    auto bytes = operand.getBytes();
    TS_ASSERT_EQUALS(bytes[0], 0x32);
    TS_ASSERT_EQUALS(bytes[1], 0x30);
  }

  void test_getWildcardSteps() {
    string s = "w:20";
    SubCommand subCmd(s);
    auto steps = subCmd.getWildcardSteps();
    TS_ASSERT_EQUALS(steps, 20);
  }

  void test_initialize() {
    string s = "i8:1";
    SubCommand subCmd1(s);
    TS_ASSERT_EQUALS(subCmd1.op, ScanParser::OpType::Eq);

    s = "i8:~ 2";
    SubCommand subCmd2(s);
    TS_ASSERT_EQUALS(subCmd2.op, ScanParser::OpType::Around);
  }
};
