#ifndef MED_TYPES_HPP
#define MED_TYPES_HPP

#include <string>
#include <vector>
using namespace std;

typedef unsigned long MemAddr;
typedef unsigned char Byte;

struct Bytes { // TODO: Consider convert to class
  Bytes(Byte* data, int size) : data(data), size(size) {}
  Byte* data;
  int size;
};

struct ProcMaps {
  vector<MemAddr> starts;
  vector<MemAddr> ends;
};

struct Process {
  string pid;
  string cmdline; //aka "process" in GUI
};


enum ScanType {
  Int8,
  Int16,
  Int32,
  Float32,
  Float64,
  Unknown
};

const string SCAN_TYPE_INT_8 = "int8";
const string SCAN_TYPE_INT_16 = "int16";
const string SCAN_TYPE_INT_32 = "int32";
const string SCAN_TYPE_FLOAT_32 = "float32";
const string SCAN_TYPE_FLOAT_64 = "float64";

#endif
