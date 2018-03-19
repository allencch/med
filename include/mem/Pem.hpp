#include "mem/Mem.hpp"
#include "mem/MemIO.hpp"

// This is Pem (Process mEMory). Derived from Mem
// Basically, Pem doesn't store the value.
// Value always read from MemIO.
// As a result, it doesn't really matter scanType change to string
class Pem : public Mem {
public:
  explicit Pem(size_t size, MemIO* memio);
  Pem(Address addr, size_t size, MemIO* memio);
  ~Pem();

  string getValue(const string& scanType);
  string getValue();
  string getScanType();
  void setValue(const string& value, const string& scanType);
  void setScanType(const string& scanType);

  void rememberValue(const string& value, const string& scanType);
  string recallValue(const string& scanType);

  MemIO* getMemIO();
  static string bytesToString(Byte* value, const string& scanType);
  static tuple<Byte*, size_t> stringToBytes(const string& value, const string& scanType);

  static std::shared_ptr<Pem> convertToPemPtr(MemPtr mem, MemIO* memio);

private:
  ScanType scanType;
  MemIO* memio;
  Byte* rememberedValue;
};

typedef std::shared_ptr<Pem> PemPtr;
