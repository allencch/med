#include "mem/Pem.hpp"
#include "mem/MemIO.hpp"

// This is Sem (Saved/stored process mEMory). Derived from Pem
class Sem : public Pem {
public:
  explicit Sem(PemPtr pem);
  Sem(Sem& sem);
  Sem(size_t size, MemIO* memio);
  Sem(Address addr, size_t size, MemIO* memio);
  bool isLocked();
  void lock(bool value);
  string getDescription();
  void setDescription(string s);

  void setLockedValue(string s);
  string& getLockedValue();
  void lockValue();

  static std::shared_ptr<Sem> clone(shared_ptr<Sem> semPtr);
  static std::shared_ptr<Sem> convertToSemPtr(PemPtr);

private:
  bool locked;
  string description;
  string lockedValue;
};

typedef std::shared_ptr<Sem> SemPtr;
