#include <string>
#include <cxxtest/TestSuite.h>

#include "med/MedCommon.hpp"

class TestMedCommon : public CxxTest::TestSuite {
public:
  void testHexStrToInt() {
    string s = "a";
    int res = hexStrToInt(s);
    TS_ASSERT_EQUALS(res, 10);
  }
};
