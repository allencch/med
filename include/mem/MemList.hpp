#include <vector>
#include "mem/Mem.hpp"

using namespace std;

class MemList {
public:
  explicit MemList(vector<MemPtr> list);
  size_t size();
  string getAddress(int index);
  string getValue(int index, const string& scanType);
  string getValue(int index);
  void dump(int index, bool newline = true);
private:
  vector<MemPtr> list;
};
