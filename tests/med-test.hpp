#include <string>
#include <cxxtest/TestSuite.h>

#include "MemOperator.hpp"
#include "ScanParser.hpp"

using namespace std;

class TestScanParser : public CxxTest::TestSuite {
 public:
  void testTrim(void) {
    string s = " foo bar   ";
    string res = ScanParser::trim(s);
    TS_ASSERT_EQUALS(res, "foo bar");
    TS_ASSERT_DIFFERS(res, "foo bar ");
  }
  void testGetOp() {
    string s = "=1234";
    string op = ScanParser::getOp(s);
    TS_ASSERT_EQUALS(op, "=");

    s = "> 1234";
    op = ScanParser::getOp(s);
    TS_ASSERT_EQUALS(op, ">");

    s = ">=1234";
    op = ScanParser::getOp(s);
    TS_ASSERT_EQUALS(op, ">=");

    s = "!1234";
    op = ScanParser::getOp(s);
    TS_ASSERT_EQUALS(op, "!");
    s = "<=1234";
    op = ScanParser::getOp(s);
    TS_ASSERT_EQUALS(op, "<=");
    
    s = "<1234";
    op = ScanParser::getOp(s);
    TS_ASSERT_EQUALS(op, "<");

    s = "1234";
    op = ScanParser::getOp(s);
    TS_ASSERT_EQUALS(op, "");

    TS_ASSERT_EQUALS(ScanParser::getOp(" <> 1234, 5432 "), "<>");
  }

  void testStringToOpType() {
    TS_ASSERT_EQUALS(ScanParser::stringToOpType("<"), ScanParser::Lt);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType(">"), ScanParser::Gt);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType("<="), ScanParser::Le);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType(">="), ScanParser::Ge);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType("!"), ScanParser::Neq);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType("="), ScanParser::Eq);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType(""), ScanParser::Eq);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType("<>"), ScanParser::Within);
  }

  void testGetValue() {
    TS_ASSERT_EQUALS(ScanParser::getValue("<123"), "123");
    TS_ASSERT_EQUALS(ScanParser::getValue("!4321"), "4321");
    TS_ASSERT_EQUALS(ScanParser::getValue(">=4321"), "4321");
    TS_ASSERT_EQUALS(ScanParser::getValue("4321"), "4321");
    TS_ASSERT_EQUALS(ScanParser::getValue("  >=   4321.1234   "), "4321.1234");
  }

  void testIsArray() {
    TS_ASSERT_EQUALS(ScanParser::isArray("1,2,3"), true);
    TS_ASSERT_EQUALS(ScanParser::isArray("1 2 3"), false);
  }

  void testGetValues() {
    string s = "<> 1 , 2 , 3  ";
    vector<string> values = ScanParser::getValues(s);
    TS_ASSERT_EQUALS(values.size(), 3);
    TS_ASSERT_EQUALS(values[2], "3");

    s = "  > 10";
    values = ScanParser::getValues(s);
    TS_ASSERT_EQUALS(values.size(), 1);
    TS_ASSERT_EQUALS(values[0], "10");
  }
};

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
};
