#include <vector>
#include <utility>

#include "med/MedTypes.hpp"

using namespace std;

class Maps {
public:
  Maps();
  AddressPairs& getMaps();
  bool hasPair(const AddressPair& pair);
  void push(const AddressPair& pair);
  size_t size();

private:
  AddressPairs maps;
};
