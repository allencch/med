#ifndef SCAN_PARAMS_HPP
#define SCAN_PARAMS_HPP

#include <mutex>
#include <string>
#include <vector>
#include "med/MedTypes.hpp"
#include "med/Operands.hpp"
#include "mem/MemIO.hpp"
#include "mem/Maps.hpp"

using namespace std;

struct ScanParams {
  MemIO* memio;
  std::mutex& mutex;
  vector<MemPtr>& list;
  Maps& maps;
  size_t mapIndex;
  int fd;
  std::mutex& fdMutex;
  Operands& operands;
  int size;
  const string& scanType;
  const ScanParser::OpType& op;
  bool fastScan = false;
  int lastDigit = -1;
};

#endif
