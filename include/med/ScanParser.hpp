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

class ScanParser {
public:
  static constexpr const char* OP_REGEX = "^(=|>(?=[^=])|<(?=[^=>])|>=|<=|!|<>|\\?|<|>)";
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
  static string getOp(const string &v);
  static string trim(const string &s);
  static OpType stringToOpType(const string &s);
  static OpType getOpType(const string &v);
  static string getValue(const string &v);
  static vector<string> getValues(const string &v);
  static bool hasValues(const string& v);
  static bool isArray(const string &v);

  static bool isValid(const string &v);
  static bool isSnapshotOperator(const OpType& opType);

  static vector<string> split(const string &s, char delim);

  static tuple<Byte*, size_t> valueToBytes(const string& v, const string& t);

private:
  static tuple<Byte*, size_t> numericToBytes(const string& v, const string& t);
  static tuple<Byte*, size_t> stringToBytes(const string& v);
};

#endif
