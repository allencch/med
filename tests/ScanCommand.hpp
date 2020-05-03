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
};
