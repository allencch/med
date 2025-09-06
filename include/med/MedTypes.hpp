#ifndef MED_TYPES_HPP
#define MED_TYPES_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

using namespace std;

typedef uint8_t Byte;
typedef unsigned long Address;

enum ScanType {
  Int8,
  Int16,
  Int32,
  Int64,
  Float32,
  Float64,
  String,
  Custom,
  Ptr32,
  Ptr64,
  Unknown
};


// To accommodate double
// https://stackoverflow.com/questions/1701055/what-is-the-maximum-length-in-chars-needed-to-represent-any-double-value
const int MAX_STRING_SIZE = 1079;

const string SCAN_TYPE_INT_8 = "int8";
const string SCAN_TYPE_INT_16 = "int16";
const string SCAN_TYPE_INT_32 = "int32";
const string SCAN_TYPE_INT_64 = "int64";
const string SCAN_TYPE_FLOAT_32 = "float32";
const string SCAN_TYPE_FLOAT_64 = "float64";
const string SCAN_TYPE_STRING = "string";
const string SCAN_TYPE_CUSTOM = "custom";
const string SCAN_TYPE_PTR_32 = "ptr32";
const string SCAN_TYPE_PTR_64 = "ptr64";
const string SCAN_TYPE_UNKNOWN = "unknown";

enum EncodingType {
  Default,
  Big5
};

typedef pair<Address, Address> AddressPair;
typedef vector<AddressPair> AddressPairs;

typedef std::shared_ptr<Byte[]> BytePtr;

namespace ScanParser {
  enum OpType {
    Eq,
    Gt,
    Lt,
    Neq,
    Ge,
    Le,
    Within,
    Around,
    SnapshotSave
  };
};

typedef vector<int> Integers;

#endif
