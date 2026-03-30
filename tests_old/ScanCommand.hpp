#include <string>
#include <cxxtest/TestSuite.h>

#include "med/ScanCommand.hpp"

using namespace std;

class TestScanCommand : public CxxTest::TestSuite {
public:
  void test_getSize() {
    string s = "1";
    ScanCommand scanCommand(s);
    auto size = scanCommand.getSize();
    TS_ASSERT_EQUALS(size, 4);

    s = "s:'20'";
    ScanCommand scanCommand2(s);
    size = scanCommand2.getSize();
    TS_ASSERT_EQUALS(size, 2);

    s = "w:5";
    ScanCommand scanCommand3(s);
    size = scanCommand3.getSize();
    TS_ASSERT_EQUALS(size, 5);

    s = "s:'2', w:5, s:'3'";
    ScanCommand scanCommand4(s);
    size = scanCommand4.getSize();
    TS_ASSERT_EQUALS(size, 7);
  }

  void test_multiArounds() {
    string s = "~ 1, ~ 2";
    ScanCommand scanCommand(s, SCAN_TYPE_FLOAT_64);
    auto size = scanCommand.getSize();
    TS_ASSERT_EQUALS(size, 16);

    TS_ASSERT_EQUALS(scanCommand.getFirstScanType(), SCAN_TYPE_FLOAT_64);

    auto subCommands = scanCommand.getSubCommands();
    TS_ASSERT_EQUALS(subCommands[0].getSize(), 8);
    TS_ASSERT_EQUALS(subCommands[0].op, ScanParser::OpType::Around);
    TS_ASSERT_EQUALS(subCommands[1].getSize(), 8);
    TS_ASSERT_EQUALS(subCommands[1].op, ScanParser::OpType::Around);
  }
};
