#include <string>
#include <cxxtest/TestSuite.h>

#include "med/ScanParser.hpp"
#include "mem/StringUtil.hpp"

using namespace std;

class TestScanParser : public CxxTest::TestSuite {
public:
  void testTrim(void) {
    string s = " foo bar   ";
    string res = StringUtil::trim(s);
    TS_ASSERT_EQUALS(res, "foo bar");
    TS_ASSERT_DIFFERS(res, "foo bar ");
  }

  void testSnapshotGetOp() {
    TS_ASSERT_EQUALS(ScanParser::getOp(">"), ">");
    TS_ASSERT_EQUALS(ScanParser::getOp("<"), "<");
    TS_ASSERT_EQUALS(ScanParser::getOp("!"), "!");
    TS_ASSERT_EQUALS(ScanParser::getOp("="), "=");
    TS_ASSERT_EQUALS(ScanParser::getOp("?"), "?");
  }

  void testStringToOpType() {
    TS_ASSERT_EQUALS(ScanParser::stringToOpType("<"), ScanParser::Lt);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType(">"), ScanParser::Gt);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType("!"), ScanParser::Neq);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType("="), ScanParser::Eq);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType(""), ScanParser::Eq);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType("?"), ScanParser::SnapshotSave);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType(">="), ScanParser::Ge);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType("<="), ScanParser::Le);
    TS_ASSERT_EQUALS(ScanParser::stringToOpType("<>"), ScanParser::Within);
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

    TS_ASSERT_EQUALS(ScanParser::getOp(" <> 1234 5432 "), "<>");

    TS_ASSERT_EQUALS(ScanParser::getOp(" < "), "<");
  }

  void testGetValue() {
    TS_ASSERT_EQUALS(ScanParser::getValue("<123"), "123");
    TS_ASSERT_EQUALS(ScanParser::getValue("!4321"), "4321");
    TS_ASSERT_EQUALS(ScanParser::getValue(">=4321"), "4321");
    TS_ASSERT_EQUALS(ScanParser::getValue("4321"), "4321");
    TS_ASSERT_EQUALS(ScanParser::getValue("  >=   4321.1234   "), "4321.1234");
    TS_ASSERT_EQUALS(ScanParser::getValue("<"), "");
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

  void test_ValueToOperands_to_string() {
    string s = "1";
    Operands result = ScanParser::valueToOperands(s, SCAN_TYPE_STRING);
    TS_ASSERT_EQUALS(result.count(), 1);
    auto operand = result.getFirstOperand();

    TS_ASSERT_EQUALS(operand.getSize(), 1);
    Byte* bytes = operand.getBytes();
    TS_ASSERT_EQUALS(bytes[0], 49);

    s = "100";
    result = ScanParser::valueToOperands(s, SCAN_TYPE_STRING);
    TS_ASSERT_EQUALS(result.count(), 1);
    operand = result.getFirstOperand();

    TS_ASSERT_EQUALS(operand.getSize(), 3);
    bytes = operand.getBytes();
    TS_ASSERT_EQUALS(bytes[0], 49);
    TS_ASSERT_EQUALS(bytes[1], 48);
    TS_ASSERT_EQUALS(bytes[2], 48);
  }

  void test_valueToOperands_to_numeric() {
    string s = "1";
    Operands result = ScanParser::valueToOperands(s, SCAN_TYPE_INT_32);
    TS_ASSERT_EQUALS(result.count(), 1);
    auto operand = result.getFirstOperand();

    TS_ASSERT_EQUALS(operand.getSize(), 4);
    Byte* bytes = operand.getBytes();
    TS_ASSERT_EQUALS(bytes[0], 1);
    TS_ASSERT_EQUALS(bytes[1], 0);
    TS_ASSERT_EQUALS(bytes[2], 0);
    TS_ASSERT_EQUALS(bytes[3], 0);

    s = "> 10";
    result = ScanParser::valueToOperands(s, SCAN_TYPE_INT_32);
    TS_ASSERT_EQUALS(result.count(), 1);
    operand = result.getFirstOperand();

    TS_ASSERT_EQUALS(operand.getSize(), 4);
    bytes = operand.getBytes();
    TS_ASSERT_EQUALS(bytes[0], 10);
    TS_ASSERT_EQUALS(bytes[1], 0);
    TS_ASSERT_EQUALS(bytes[2], 0);
  }

  void test_getTwoOperands() {
    string s = "<> 1 200";
    Operands result = ScanParser::getTwoOperands(s, SCAN_TYPE_INT_32);
    TS_ASSERT_EQUALS(result.count(), 2);

    auto operand = result.getFirstOperand();
    TS_ASSERT_EQUALS(operand.getSize(), 4);
    Byte* bytes = operand.getBytes();
    TS_ASSERT_EQUALS(bytes[0], 1);
    TS_ASSERT_EQUALS(bytes[1], 0);
    TS_ASSERT_EQUALS(bytes[2], 0);
    TS_ASSERT_EQUALS(bytes[3], 0);

    operand = result.getSecondOperand();
    TS_ASSERT_EQUALS(operand.getSize(), 4);
    bytes = operand.getBytes();
    TS_ASSERT_EQUALS(bytes[0], 200);
    TS_ASSERT_EQUALS(bytes[1], 0);
    TS_ASSERT_EQUALS(bytes[2], 0);
    TS_ASSERT_EQUALS(bytes[3], 0);
  }
};
