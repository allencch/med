/**
 * This file intends to parse the scan string, to reduce the UI design
 */
#ifndef SCAN_PARSER_H
#define SCAN_PARSER_H

#include <string>
#include <vector>

#include "MedTypes.hpp"

using namespace std;

class ScanParser {
public:
  enum OpType {
    Eq,
    Gt,
    Lt,
    Neq,
    Ge,
    Le,
    Within
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

  static vector<string> split(const string &s, char delim);
};

#endif
