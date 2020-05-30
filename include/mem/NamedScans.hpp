#ifndef NAMED_SCANS_HPP
#define NAMED_SCANS_HPP

#include <map>
#include <string>
#include "mem/MemList.hpp"

using namespace std;

class NamedScans {
public:
  static const string DEFAULT;

  NamedScans();
  MemList* addNewScan(string name);
  MemList* getMemList(string name = DEFAULT);
  bool remove(string name);

  void setActiveName(string name);
  string getActiveName();

private:
  map<string, MemList> data;
  string activeName;
};

#endif
