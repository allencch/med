/**
 * This file intends to parse the scan string, to reduce the UI design
 */
#ifndef SCAN_PARSER_H
#define SCAN_PARSER_H

#include <string>
#include <vector>

#include "med/MedTypes.hpp"
#include "med/Bytes.hpp"

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
  static Bytes getBytes(const string &v, const string& t); /**<< Remember to delete[] */
  static bool isArray(const string &v);

  static bool isValid(const string &v);
  static bool isSnapshotOperator(const OpType& opType);

  static vector<string> split(const string &s, char delim);

private:
  static Bytes getNumericBytes(const string& v, const string& t);
  static Bytes getStringBytes(const string& v);
};

#endif
