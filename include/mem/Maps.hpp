#include <vector>
#include <utility>

#include "med/MedTypes.hpp"

using namespace std;

typedef pair<Address, Address> AddressPair;
typedef vector<AddressPair> AddressPairs;

class Maps {
public:
  Maps();
  AddressPairs& getMaps();
  bool hasPair(const AddressPair& pair);
  void push(const AddressPair& pair);

  ProcMaps toProcMaps(); // @deprecated
private:
  AddressPairs maps;
};
