#include <cstring>
#include <iostream>

#include "med/MedTypes.hpp"
#include "med/ByteManager.hpp"

using namespace std;

int main() {
  ByteManager* byteManager = &ByteManager::getInstance();

  Byte* data = byteManager->newByte(5);
  memset(data, 0, 5);

  ByteManager& b2 = ByteManager::getInstance();

  cout << byteManager->getRecordedBytes().size() << endl;
  cout << b2.getRecordedBytes().size() << endl;


  //byteManager->deleteByte(data);
  b2.deleteByte(data);

  // delete byteManager;
  return 0;
}
