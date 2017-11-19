#ifndef MED_SCAN_H
#define MED_SCAN_H

#include <string>

#include "med/MedTypes.hpp"
#include "med/Bytes.hpp"
#include "med/ScanParser.hpp"

using namespace std;

/**
 * This is the Scanner replacement
 */
class MedScan {
public:
  MedScan();
  MedScan(MemAddr address);
  MemAddr address;

  void setAddress(MemAddr address);
  MemAddr getAddress();

  string getScanType();
  void setScanType(string s);
  ScanType scanType; //direct access

  //No value, because value is calculated based on scan type
  string getValue(long pid);
  string getValue(long pid, string scanType);
  void setValue(long pid, string val);

  Bytes* getValueAsNewBytes(long pid, ScanType scanType);
};

#endif
