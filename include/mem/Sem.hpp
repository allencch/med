#include "mem/Pem.hpp"
#include "mem/MemIO.hpp"

// This is Sem (Saved/stored process mEMory). Derived from Pem
class Sem : public Pem {
public:
  explicit Sem(PemPtr pem);
  Sem(size_t size, MemIO* memio);
  Sem(Address addr, size_t size, MemIO* memio);
  bool isLocked();
  void lock(bool value);
  string getDescription();
  void setDescription(string s);

private:
  bool locked;
  string description;
};

typedef std::shared_ptr<Sem> SemPtr;
