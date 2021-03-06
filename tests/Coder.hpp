#include <string>
#include <cxxtest/TestSuite.h>
#include <unicode/uclean.h>

#include "med/Coder.hpp"

using namespace std;

class TestCoder : public CxxTest::TestSuite {
public:
  void testConvertBig5ToUtf8() {
    char buffer[255];
    // Big5 臺灣 1472614331
    buffer[0] = 0xbb;
    buffer[1] = 0x4f;
    buffer[2] = 0xc6;
    buffer[3] = 0x57;
    buffer[4] = 0;

    // Utf8 12224488
    char expected[255];
    expected[0] = 0xe8;
    expected[1] = 0x87;
    expected[2] = 0xba;
    expected[3] = 0xe7;
    expected[4] = 0x81;
    expected[5] = 0xa3;
    expected[6] = 0;

    string output = convertBig5ToUtf8(buffer);
    TS_ASSERT_EQUALS(strcmp(output.c_str(), expected), 0);

    u_cleanup(); // To avoid valgrind memory leak report
  }
};
