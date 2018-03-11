#ifndef MEM_MANAGER_HPP
#define MEM_MANAGER_HPP

#include <vector>

#include "mem/Mem.hpp"

using namespace std;

class MemManager {
public:
  MemManager();
  vector<MemPtr>& getMems();
  void setMems(const vector<MemPtr>& mems);
  void clear();

private:
  vector<MemPtr> mems;
};

#endif
