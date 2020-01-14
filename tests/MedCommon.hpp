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

  void testGetPidStatus() {
    char pid[] = "351443 (Puzzle Quest.ex) T 351403 351365";
    char res = getPidStatus(pid);
    TS_ASSERT_EQUALS(res, 'T');
  }
};
