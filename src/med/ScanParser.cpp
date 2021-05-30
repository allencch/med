#include <string>
#include <regex>
#include <vector>
#include <sstream>
#include <iostream>

#include "med/ScanParser.hpp"
#include "med/MedException.hpp"
#include "med/MedCommon.hpp"
#include "med/SubCommand.hpp"
#include "mem/StringUtil.hpp"

using namespace std;

string ScanParser::getOp(const string &v) {
  string value = StringUtil::trim(v);
  regex r(OP_REGEX);
  auto begin = sregex_iterator(value.begin(), value.end(), r);
  auto end = sregex_iterator();

  string matched;
  for (sregex_iterator i = begin; i != end; ++i) {
    smatch m = *i;
    matched = m.str();
  }
  return matched;
}

ScanParser::OpType ScanParser::stringToOpType(const string &s) {
  if (s == "?")
    return ScanParser::SnapshotSave;
  else if (s == "<")
    return ScanParser::Lt;
  else if (s == ">")
    return ScanParser::Gt;
  else if (s == "=")
    return ScanParser::Eq;
  else if (s == "!")
    return ScanParser::Neq;
  else if (s == "<=")
    return ScanParser::Le;
  else if (s == ">=")
    return ScanParser::Ge;
  else if (s == "<>")
    return ScanParser::Within;
  else if (s == "~")
    return ScanParser::Around;
  return ScanParser::Eq;
}

string ScanParser::getValue(const string &v) {
  string value = StringUtil::trim(v);
  regex r(OP_REGEX);
  string result = StringUtil::trim(regex_replace(value, r, ""));
  return result;
}

bool ScanParser::isArray(const string &v, char delimiter) {
  string value = StringUtil::trim(v);
  if (value.find(delimiter) != string::npos)
    return true;
  return false;
}

ScanParser::OpType ScanParser::getOpType(const string &v) {
  return stringToOpType(getOp(v));
}

vector<string> ScanParser::getValues(const string &v, char delimiter) {
  string value = ScanParser::getValue(v);
  vector<string> values = StringUtil::split(value, delimiter);
  return values;
}

bool ScanParser::hasValues(const string& v) {
  auto values = getValues(v);
  return values.size() > 0;
}

bool ScanParser::isValid(const string &v) {
  if (ScanParser::getOpType(v) == ScanParser::Within &&
      !ScanParser::isArray(v, ' '))
    return false;
  return true;
}

// TODO: Similar to Pem's method. Refactoring
SizedBytes ScanParser::valueToBytes(const string& v, const string& t) {
  if (t == SCAN_TYPE_STRING) {
    return ScanParser::stringToBytes(v);
  }
  else {
    return ScanParser::numericToBytes(v, t);
  }
}

SizedBytes ScanParser::numericToBytes(const string& v, const string& t) {
  vector<string> values = getValues(v);
  if (values.size() == 0) {
    throw MedException("Scan empty string");
  }

  int valueLength = scanTypeToSize(t);
  BytePtr data(new Byte[valueLength * values.size()]);
  Byte* pointer = data.get();
  for (size_t i = 0; i < values.size(); i++) {
    stringToMemory(values[i], t, pointer);
    pointer += valueLength;
  }
  return SizedBytes(data, valueLength * values.size());
}

SizedBytes ScanParser::stringToBytes(const string& v) {
  vector<string> values = getValues(v);
  if (values.size() == 0) {
    throw MedException("Scan empty string");
  }
  int valueLength = v.size();

  BytePtr data(new Byte[valueLength]);

  char* pointer = (char*)data.get();
  char buf[2];
  for (int i = 0; i < (int)v.size(); i++, pointer++) {
    sprintf(buf, "%c", v[i]);
    memcpy(pointer, buf, 1);
  }

  return SizedBytes(data, valueLength);
}

bool ScanParser::isSnapshotOperator(const OpType& opType) {
  if (opType == SnapshotSave ||
      opType == Gt ||
      opType == Lt ||
      opType == Eq ||
      opType == Neq) {
    return true;
  }
  return false;
}

Operands ScanParser::valueToOperands(const string& v, const string& t, OpType op) {
  SizedBytes bytes;
  if (t == SCAN_TYPE_STRING) {
    bytes = stringToBytes(v);
  } else if (op == OpType::Within ||
             op == OpType::Around) {
    return getTwoOperands(v, t, op);
  } else {
    bytes = numericToBytes(v, t);
  }
  vector<SizedBytes> vector = { bytes };
  Operands operands(vector);
  return operands;

}

Operands ScanParser::getTwoOperands(const string& v, const string& t, OpType op) {
  vector<string> values;
  if (op == OpType::Around) {
    values = convertAroundToWithinValues(v);
  }
  else {
    values = getValues(v, ' ');
  }

  if (values.size() < 2) {
      throw MedException("Operands should not be less than 2");
  }
  int length = scanTypeToSize(t);

  vector<SizedBytes> list;
  for (int i = 0; i < 2; i++) {
    BytePtr data(new Byte[length]);
    Byte* ptr = data.get();
    stringToMemory(values[i], t, ptr);

    list.push_back(SizedBytes(data, length));
  }
  return Operands(list);
}

ScanCommand ScanParser::getScanCommand(const string& v) {
  return ScanCommand(v);
}

vector<string> ScanParser::convertAroundToWithinValues(const string& v) {
  vector<string> values = getValues(v, ' ');
  double first = stod(values[0]);
  double second;
  if (values.size() < 2) {
    second = 1.0;
  }
  else {
    second = stod(values[1]);
  }
  return vector<string>{to_string(first - second), to_string(first + second)};
}
