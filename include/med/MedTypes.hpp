#ifndef MED_TYPES_HPP
#define MED_TYPES_HPP

#include <string>
#include <vector>
using namespace std;

typedef unsigned long MemAddr;
typedef unsigned char Byte;

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

#endif
