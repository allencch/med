#include "mem/NamedScans.hpp"

const string DEFAULT = "Default";

NamedScans::NamedScans() {
  data[DEFAULT] = MemList();
}

MemList* NamedScans::addNewScan(string name) {
  data[name] = MemList();
  return &data[name];
}

MemList* NamedScans::get(string name) {
  auto search = data.find(name);
  if (search != data.end()) {
    return &search->second;
  }
  return NULL;
}

bool NamedScans::remove(string name) {
  if (name == DEFAULT) return false;

  auto search = data.find(name);
  if (search != data.end()) {
    data.erase(search);
    return true;
  }
  return false;
}
