#include <map>

#include "med/MedTypes.hpp"

using namespace std;

class ByteManager {
public:
  ByteManager();
  virtual ~ByteManager();
  ByteManager(const ByteManager&) = delete; // Disable copy or assign
  static ByteManager& getInstance(); // Singleton
  Byte* newByte(int size);
  void deleteByte(Byte* byte);

  void clear();

  map<Byte*, Byte*> getRecordedBytes();
private:
  map<Byte*, Byte*> recordedBytes;
};
