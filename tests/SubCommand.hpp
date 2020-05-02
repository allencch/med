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
  }
};
