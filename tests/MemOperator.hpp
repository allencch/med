#include <string>
#include <cxxtest/TestSuite.h>

#include "med/med.hpp"
#include "med/MemOperator.hpp"

using namespace std;

class TestMemOperator : public CxxTest::TestSuite {
public:
  void testMemGt() {
    unsigned char ptr1[] = { 0x20, 0x00, 0x00 };
    unsigned char ptr2[] = { 0x10, 0x00, 0x00 };

    TS_ASSERT_EQUALS(memGt(ptr1, ptr2, sizeof(ptr1)), true);
  }
  void testMemGe() {
    unsigned char ptr1[] = { 0x20, 0x00, 0x00 };
    unsigned char ptr2[] = { 0x20, 0x00, 0x00 };

    TS_ASSERT_EQUALS(memGe(ptr1, ptr2, sizeof(ptr1)), true);

    ptr2[0] = 0x21;
    TS_ASSERT_EQUALS(memGe(ptr1, ptr2, sizeof(ptr1)), false);

    // Test endianness
    ptr1[0] = 0x00;
    ptr1[1] = 0x01;
    TS_ASSERT_EQUALS(memGe(ptr1, ptr2, sizeof(ptr1)), true);
  }
  void testMemWithin() {
    unsigned char src[] = { 0x20, 0x00, 0x00 };
    unsigned char low[] = { 0x10, 0x00, 0x00 };
    unsigned char up[] = { 0x30, 0x00, 0x00 };
    TS_ASSERT_EQUALS(memWithin(src, low, up, sizeof(src)), true);

    up[0] = 0x00;
    up[2] = 0xff;
    TS_ASSERT_EQUALS(memWithin(src, low, up, sizeof(src)), true);
  }

  void testMemCompare() {
    unsigned char src[] = { 0x20, 0x00, 0x00 };
    unsigned char low[] = { 0x10, 0x00, 0x00 };
    unsigned char up[] = { 0x30, 0x00, 0x00 };
    TS_ASSERT_EQUALS(memCompare(src, low, sizeof(src), ScanParser::Ge), true);
    TS_ASSERT_EQUALS(memCompare(src, up, sizeof(src), ScanParser::Ge), false);
  }

  void testMemCompareWithin() {
    unsigned char src[] = { 0x20, 0x00 };
    unsigned char dest[] = { 0x10, 0x00, 0x21, 0x00 };
    TS_ASSERT_EQUALS(memCompare(src, sizeof(src),
                                dest, sizeof(dest),
                                ScanParser::Within), true);

    dest[2] = 0x19;
    TS_ASSERT_EQUALS(memCompare(src, sizeof(src),
                                dest, sizeof(dest),
                                ScanParser::Within), false);
  }

  void testAddressRoundDown() {
    MemAddr addr = 0x11112222;
    MemAddr rounded = addressRoundDown(addr);
    TS_ASSERT_EQUALS(rounded, 0x11112220);

    addr = 0xaaaabbb8;
    rounded = addressRoundDown(addr);
    TS_ASSERT_EQUALS(rounded, 0xaaaabbb0);

    addr = 0xffffccc0;
    rounded = addressRoundDown(addr);
    TS_ASSERT_EQUALS(rounded, 0xffffccc0);

    addr = 0x4321432f;
    rounded = addressRoundDown(addr);
    TS_ASSERT_EQUALS(rounded, 0x43214320);
  }
};
