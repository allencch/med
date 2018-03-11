#include <cstdio>

#include "mem/MemList.hpp"
#include "med/MedCommon.hpp"

using namespace std;

MemList::MemList(vector<MemPtr> list) {
  this->list = list;
}

size_t MemList::size() {
  return list.size();
}

string MemList::getAddress(int index) {
  return list[index]->getAddressAsString();
}

// TODO: Fix this so that can by any scanType
string MemList::getValue(int index, const string& scanType) {
  int value = list[index]->getValueAsInt();
  return std::to_string(value);
}

// TODO: Fix get value based on stored scanType
string MemList::getValue(int index) {
  int value = list[index]->getValueAsInt();
  return std::to_string(value);
}

void MemList::dump(int index, bool newline) {
  list[index]->dump(newline);
}
