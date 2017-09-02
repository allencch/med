#include <string>
#include <cxxtest/TestSuite.h>

#include "med/ScanParser.hpp"

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

  void testGetBytes() {
    string s = "1";
    Bytes bytes = ScanParser::getBytes(s, "int8");
    TS_ASSERT_EQUALS(bytes.data[0], 1);
    TS_ASSERT_EQUALS(bytes.size, 1);
    delete[] bytes.data;

    s = "1,2";
    bytes = ScanParser::getBytes(s, "int8");
    TS_ASSERT_EQUALS(bytes.data[0], 1);
    TS_ASSERT_EQUALS(bytes.data[1], 2);
    TS_ASSERT_EQUALS(bytes.size, 2);
    delete[] bytes.data;

    s = "3,2";
    bytes = ScanParser::getBytes(s, "int8");
    TS_ASSERT_EQUALS(bytes.data[0], 3);
    TS_ASSERT_EQUALS(bytes.data[1], 2);
    TS_ASSERT_EQUALS(bytes.size, 2);
    delete[] bytes.data;

    s = "3, 2";
    bytes = ScanParser::getBytes(s, "int16");
    TS_ASSERT_EQUALS(bytes.data[0], 3);
    TS_ASSERT_EQUALS(bytes.data[1], 0);
    TS_ASSERT_EQUALS(bytes.data[2], 2);
    TS_ASSERT_EQUALS(bytes.data[3], 0);
    TS_ASSERT_EQUALS(bytes.size, 4);
    delete[] bytes.data;
  }
};
