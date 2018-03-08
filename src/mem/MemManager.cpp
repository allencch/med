#include "mem/MemManager.hpp"

MemManager::MemManager() {}

vector<MemPtr>& MemManager::getMems() {
  return mems;
}

void MemManager::setMems(const vector<MemPtr>& mems) {
  this->mems = mems;
}
