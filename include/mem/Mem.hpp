#ifndef MEM_H
#define MEM_H

#include <memory>
#include "med/MedTypes.hpp"

class Mem {
public:
  explicit Mem(size_t size);
  Mem(Address addr, size_t size);
  ~Mem();

  void dump(bool newline = true);
  void setValue(int value); // Deprecated

  Address getAddress();
  string getAddressAsString();
  void setAddress(Address addr);
  int getValueAsInt();

  size_t getSize();
  Byte* getData();

protected:
  Byte* data;
  size_t size;
  Address address;

private:
  void initialize(size_t size);
};

typedef std::shared_ptr<Mem> MemPtr;

#endif
