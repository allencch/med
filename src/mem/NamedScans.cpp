#include <iostream>
#include "mem/NamedScans.hpp"
#include "mem/StringUtil.hpp"
#include "med/MedTypes.hpp"

using namespace std;

NamedScans::NamedScans() {
  data[DEFAULT] = MemList();
  activeName = DEFAULT;
  scanTypes[DEFAULT] = SCAN_TYPE_INT_32;
}

MemList* NamedScans::addNewScan(string name) {
  auto trimmed = StringUtil::trim(name);
  if (!trimmed.size()) return NULL;

  auto search = data.find(trimmed);
  if (search != data.end()) {
    return NULL;
  }

  data[trimmed] = MemList();
  return &data[trimmed];
}

MemList* NamedScans::getMemList() {
  return getMemList(activeName);
}

MemList* NamedScans::getMemList(string name) {
  auto trimmed = StringUtil::trim(name);
  if (!trimmed.size()) return NULL;

  auto search = data.find(trimmed);
  if (search != data.end()) {
    return &search->second;
  }
  return NULL;
}

void NamedScans::setMemPtrs(vector<MemPtr> list, string scanType) {
  getMemList()->setList(list);
  setScanType(scanType);
}

bool NamedScans::remove(string name) {
  auto trimmed = StringUtil::trim(name);
  if (trimmed == DEFAULT || !trimmed.size()) return false;

  auto search = data.find(trimmed);
  if (search != data.end()) {
    data.erase(search);
    removeScanTypes(trimmed);
    activeName = DEFAULT;
    return true;
  }
  return false;
}

void NamedScans::removeScanTypes(string name) {
  auto trimmed = StringUtil::trim(name);
  auto search = scanTypes.find(trimmed);
  if (search != scanTypes.end()) {
    scanTypes.erase(search);
  }
}

string NamedScans::getActiveName() {
  return activeName;
}

void NamedScans::setActiveName(string name) {
  auto trimmed = StringUtil::trim(name);
  if (!trimmed.size()) return;
  activeName = trimmed;
}

void NamedScans::setScanType(string type) {
  scanTypes[activeName] = type;
}

string NamedScans::getScanType() {
  auto result = scanTypes[activeName];
  if (!result.length()) return SCAN_TYPE_INT_32;

  return result;
}
