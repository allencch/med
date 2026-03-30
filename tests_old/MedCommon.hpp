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

  void test_stringToMemory() {
    string t = "int16";
    int valueLength = scanTypeToSize(t);
    Byte* buffer = new Byte[valueLength];

    stringToMemory("100", t, buffer);
    TS_ASSERT_EQUALS(buffer[0], 100);
    TS_ASSERT_EQUALS(buffer[1], 0);

    stringToMemory("0x63", t, buffer);
    TS_ASSERT_EQUALS(buffer[0], 99);
    TS_ASSERT_EQUALS(buffer[1], 0);

    delete[] buffer;
  }
};
