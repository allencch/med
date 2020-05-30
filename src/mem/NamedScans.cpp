#include "mem/NamedScans.hpp"

const string NamedScans::DEFAULT = "Default";

NamedScans::NamedScans() {
  data[DEFAULT] = MemList();
  activeName = DEFAULT;
}

MemList* NamedScans::addNewScan(string name) {
  data[name] = MemList();
  return &data[name];
}

MemList* NamedScans::getMemList(string name) {
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
    activeName = DEFAULT;
    return true;
  }
  return false;
}

string NamedScans::getActiveName() {
  return activeName;
}

void NamedScans::setActiveName(string name) {
  activeName = name;
}
