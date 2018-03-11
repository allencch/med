#include <vector>
#include "mem/Mem.hpp"

using namespace std;

// List is actually PemPtr
class MemList {
public:
  explicit MemList(vector<MemPtr> list);
  size_t size();
  string getAddressAsString(int index);
  Address getAddress(int index);
  string getValue(int index, const string& scanType);
  string getValue(int index);
  string getScanType(int index);
  void dump(int index, bool newline = true);
  void setValue(int index, const string& value, const string& scanType);

private:
  vector<MemPtr> list;
};
