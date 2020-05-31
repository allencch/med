#include <string>
#include <cxxtest/TestSuite.h>

#include "mem/StringUtil.hpp"

class TestStringUtil : public CxxTest::TestSuite {
public:
  void testToLower() {
    string s = "ABC";
    string res = StringUtil::toLower(s);
    TS_ASSERT_EQUALS(res, "abc");
  }

  void test_trim() {
    string s = "   A     ";
    string res = StringUtil::trim(s);
    TS_ASSERT_EQUALS(res, "A");
  }
};
