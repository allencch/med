/**
 * This file intends to parse the scan string, to reduce the UI design
 */
#ifndef SCAN_PARSER_H
#define SCAN_PARSER_H

#include <string>

using namespace std;

class ScanParser {
public:
  enum OpType {
    Eq,
    Gt,
    Lt,
    Neq,
    Ge,
    Le
  };
  static string getOp(string v);
  static string trim(string s);
  static OpType stringToOpType(string s);
  static OpType getOpType(string v);
  static string getValue(string v);
  static bool isArray(string v);
};

#endif
