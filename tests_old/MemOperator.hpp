#include <string>
#include <cxxtest/TestSuite.h>

#include "med/MemOperator.hpp"
#include "med/Operands.hpp"

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
    Address addr = 0x11112222;
    Address rounded = addressRoundDown(addr);
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

  void test_compare_operands() {
    int length = 4;
    SizedBytes sizedBytes = SizedBytes::create(length);
    Operands operands(std::vector<SizedBytes>{ sizedBytes });
    Byte* ptr = sizedBytes.getBytes();
    ptr[0] = 1;
    ptr[1] = 0;
    ptr[2] = 0;
    ptr[3] = 0;

    Byte* bytes = new Byte[length];
    ptr = bytes;
    ptr[0] = 1;
    ptr[1] = 0;
    ptr[2] = 0;
    ptr[3] = 0;

    bool result = memCompare(bytes, length, operands, ScanParser::Eq);
    TS_ASSERT_EQUALS(result, true);

    ptr[0] = 2;
    result = memCompare(bytes, length, operands, ScanParser::Neq);
    TS_ASSERT_EQUALS(result, true);

    // Target bytes is greater than input value
    result = memCompare(bytes, length, operands, ScanParser::Gt);
    TS_ASSERT_EQUALS(result, true);

    delete[] bytes;
  }

  void test_compare_two_operands() {
    int length = 4;
    SizedBytes first = SizedBytes::create(length);
    Byte* ptr = first.getBytes();
    ptr[0] = 10;
    ptr[1] = 0;
    ptr[2] = 0;
    ptr[3] = 0;

    SizedBytes second = SizedBytes::create(length);
    ptr = second.getBytes();
    ptr[0] = 20;
    ptr[1] = 0;
    ptr[2] = 0;
    ptr[3] = 0;

    Operands operands(std::vector<SizedBytes>{ first, second });

    Byte* bytes = new Byte[length];
    ptr = bytes;
    ptr[0] = 10;
    ptr[1] = 0;
    ptr[2] = 0;
    ptr[3] = 0;

    bool result = memCompare(bytes, length, operands, ScanParser::Within);
    TS_ASSERT_EQUALS(result, true);

    ptr[0] = 9;
    result = memCompare(bytes, length, operands, ScanParser::Within);
    TS_ASSERT_EQUALS(result, false);

    ptr[0] = 20;
    result = memCompare(bytes, length, operands, ScanParser::Within);
    TS_ASSERT_EQUALS(result, true);

    ptr[0] = 21;
    result = memCompare(bytes, length, operands, ScanParser::Within);
    TS_ASSERT_EQUALS(result, false);

    delete[] bytes;
  }
};
