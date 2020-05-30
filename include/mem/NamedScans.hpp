#ifndef NAMED_SCANS_HPP
#define NAMED_SCANS_HPP

#include <map>
#include <string>
#include <vector>
#include "mem/MemList.hpp"

using namespace std;

class NamedScans {
public:
  inline static const string DEFAULT = "Default";

  NamedScans();
  MemList* addNewScan(string name);
  MemList* getMemList();
  MemList* getMemList(string name);
  void setMemPtrs(vector<MemPtr> list, string scanType);
  bool remove(string name);

  void setActiveName(string name);
  string getActiveName();

  void setScanType(string type);
  string getScanType();

private:
  void removeScanTypes(string name);
  map<string, MemList> data;
  string activeName;
  map<string, string> scanTypes;
};

#endif
