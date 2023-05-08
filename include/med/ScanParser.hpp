/**
 * This file intends to parse the scan string, to reduce the UI design
 */
#ifndef SCAN_PARSER_H
#define SCAN_PARSER_H

#include <string>
#include <vector>
#include <utility>

#include "med/MedTypes.hpp"
#include "med/SizedBytes.hpp"
#include "med/Operands.hpp"
#include "med/ScanCommand.hpp"

using namespace std;

namespace ScanParser {
  constexpr const char* OP_REGEX = "^(=|>(?=[^=])|<(?=[^=>])|>=|<=|!|<>|\\?|<|>|~)";
  string getOp(const string &v);
  OpType stringToOpType(const string &s); /** Convert a string (or character) to OpType */
  OpType getOpType(const string &v); /** Get OpType from a string */
  string getValue(const string &v);
  vector<string> getValues(const string &v, char delimiter = ',');
  bool hasValues(const string& v);
  bool isArray(const string &v, char delimiter = ',');

  bool isValid(const string &v);
  bool isSnapshotOperator(const OpType& opType);

  SizedBytes valueToBytes(const string& v, const string& t);

  SizedBytes numericToBytes(const string& v, const string& t);
  SizedBytes stringToBytes(const string& v);

  // Convert input string (value) to Operands.
  // If it is scan Within, then it will have two operands.
  // Others will consider as one operand.
  // If the input is an array (with commas), it is one operand.
  Operands valueToOperands(const string& v, const string& t, OpType op = OpType::Eq);
  Operands getTwoOperands(const string& v, const string& t, OpType op = OpType::Within);

  ScanCommand getScanCommand(const string& v, const string& scanType = SCAN_TYPE_CUSTOM);

  vector<string> convertAroundToWithinValues(const string& v);

  Integers getIntegers(const string &v, char delimiter = ',');
};

#endif
