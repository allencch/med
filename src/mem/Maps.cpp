#include <algorithm>
#include "mem/Maps.hpp"

using namespace std;

Maps::Maps() {}

AddressPairs& Maps::getMaps() {
  return maps;
}

bool Maps::hasPair(const AddressPair& pair) {
  auto it = find_if(maps.begin(), maps.end(), [&pair](AddressPair item) {
      return std::get<0>(item) >= std::get<0>(pair) && std::get<1>(item) <= std::get<1>(pair);
    });
  return it != maps.end();
}

void Maps::push(const AddressPair& pair) {
  maps.push_back(pair);
}

size_t Maps::size() {
  return maps.size();
}
