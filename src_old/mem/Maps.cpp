#include <algorithm>
#include <iostream>
#include "med/MedException.hpp"
#include "mem/Maps.hpp"

using namespace std;

Maps::Maps() {}

AddressPair& Maps::operator[](size_t index) {
  if (index >= size()) {
    throw MedException("Maps out of index");
  }
  return maps[index];
}

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

void Maps::clear() {
  maps.clear();
}

void Maps::trimByScope(const AddressPair& scope) {
  auto start = scope.first;
  auto end = scope.second;

  AddressPairs newMaps;
  for (size_t i = 0; i < size(); i++) {
    auto first = maps[i].first;
    auto second = maps[i].second;

    if (end < first || start > second) {
      continue;
    }

    if (start < first) {
      if (end < second) {
        newMaps.push_back(AddressPair(first, end));
      }
      else {
        newMaps.push_back(AddressPair(first, second));
      }
    }
    else { // start >= first
      if (end < second) {
        newMaps.push_back(AddressPair(start, end));
        break;
      }
      else {
        newMaps.push_back(AddressPair(start, second));
      }
    }
  }

  maps = newMaps;
}
