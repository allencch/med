/**
 * This file intends to parse the scan string, to reduce the UI design
 */
#ifndef SCAN_PARSER_H
#define SCAN_PARSER_H

#include <string>
#include <vector>
#include <tuple>

#include "med/MedTypes.hpp"

using namespace std;

namespace ScanParser {
  constexpr const char* OP_REGEX = "^(=|>(?=[^=])|<(?=[^=>])|>=|<=|!|<>|\\?|<|>)";
  enum OpType {
    Eq,
    Gt,
    Lt,
    Neq,
    Ge,
    Le,
    Within,
    SnapshotSave
  };
  string getOp(const string &v);
  OpType stringToOpType(const string &s);
  OpType getOpType(const string &v);
  string getValue(const string &v);
  vector<string> getValues(const string &v);
  bool hasValues(const string& v);
  bool isArray(const string &v);

  bool isValid(const string &v);
  bool isUnknownOperator(const OpType& opType);
  bool isSnapshotOperator(const OpType& opType); // @deprecated

  tuple<Byte*, size_t> valueToBytes(const string& v, const string& t);

  tuple<Byte*, size_t> numericToBytes(const string& v, const string& t);
  tuple<Byte*, size_t> stringToBytes(const string& v);
};

#endif
