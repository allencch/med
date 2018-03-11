#include "mem/Mem.hpp"
#include "mem/MemIO.hpp"

// This is Pem (Process mEMory). Derived from Mem
class Pem : public Mem {
public:
  explicit Pem(size_t size, MemIO* memio);
  Pem(Address addr, size_t size, MemIO* memio);
  string getValue(const string& scanType);
  string getScanType();
  void setValue(const string& value, const string& scanType);
  void setScanType(const string& scanType);
  bool isLocked();
  void lock(bool value);

private:
  static string bytesToString(Byte* value, const string& scanType);
  static tuple<Byte*, size_t> stringToBytes(const string& value, const string& scanType);
  ScanType scanType;
  MemIO* memio;
  bool locked;
};

typedef std::shared_ptr<Pem> PemPtr;
