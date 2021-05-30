#ifndef MAPS_HPP
#define MAPS_HPP

#include <vector>
#include <utility>

#include "med/MedTypes.hpp"

using namespace std;

class Maps {
public:
  Maps();
  AddressPair& operator[](size_t index);

  AddressPairs& getMaps();
  bool hasPair(const AddressPair& pair);
  void push(const AddressPair& pair);
  void trimByScope(const AddressPair& scope);
  size_t size();
  void clear();

private:
  AddressPairs maps;
};

#endif
