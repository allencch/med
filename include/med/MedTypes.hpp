#ifndef MED_TYPES_HPP
#define MED_TYPES_HPP

#include <string>
#include <vector>

using namespace std;

typedef uint8_t Byte;
typedef unsigned long Address;

struct ProcMaps {
  vector<Address> starts;
  vector<Address> ends;
};

enum ScanType {
  Int8,
  Int16,
  Int32,
  Float32,
  Float64,
  String,
  Unknown
};

const int MAX_STRING_SIZE = 255;

const string SCAN_TYPE_INT_8 = "int8";
const string SCAN_TYPE_INT_16 = "int16";
const string SCAN_TYPE_INT_32 = "int32";
const string SCAN_TYPE_FLOAT_32 = "float32";
const string SCAN_TYPE_FLOAT_64 = "float64";
const string SCAN_TYPE_STRING = "string";
const string SCAN_TYPE_UNKNOWN = "unknown";

enum EncodingType {
  Default,
  Big5
};

#endif
