#include <map>

#include "med/MedTypes.hpp"
#include "med/ByteManager.hpp"

using namespace std;

ByteManager::ByteManager() {}

ByteManager::~ByteManager() {
  clear();
}

ByteManager& ByteManager::getInstance() {
  static ByteManager instance;
  return instance;
}

Byte* ByteManager::newByte(int size) {
  Byte* byte = new Byte[size];
  recordedBytes[byte] = byte;
  return byte;
}

void ByteManager::deleteByte(Byte* byte) {
  auto found = recordedBytes.find(byte);
  if (found != recordedBytes.end()) {
    delete[] recordedBytes[byte];
    recordedBytes.erase(found);
  }
}

void ByteManager::clear() {
  for (auto it = recordedBytes.begin(); it != recordedBytes.end(); ++it) {
    delete[] it->second;
  }
  recordedBytes.clear();
}

map<Byte*, Byte*> ByteManager::getRecordedBytes() {
  return recordedBytes;
}
