#ifndef NAMED_SCANS_HPP
#define NAMED_SCANS_HPP

#include <map>
#include <string>
#include "mem/MemList.hpp"

using namespace std;

class NamedScans {
public:
  NamedScans();
  MemList* addNewScan(string name);
  MemList* get(string name);
  bool remove(string name);

private:
  map<string, MemList> data;
};

#endif
