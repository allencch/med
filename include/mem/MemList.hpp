#include <vector>
#include "mem/Mem.hpp"

using namespace std;

// List is actually PemPtr
class MemList {
public:
  explicit MemList(vector<MemPtr> list);
  size_t size();
  void setList(const vector<MemPtr>& list);
  string getAddressAsString(int index);
  Address getAddress(int index);
  string getValue(int index, const string& scanType);
  string getValue(int index);
  string getScanType(int index);
  void dump(int index, bool newline = true);

  void setValue(int index, const string& value, const string& scanType);
  void setScanType(int index, const string& scanType);
  void setAddress(int index, const string& address);
  int getLastIndex();
  void sortByAddress();
  void clear();
  MemPtr getMemPtr(int index);
  void addMemPtr(MemPtr mem);

  static vector<MemPtr> sortByAddress(vector<MemPtr>& list);

private:
  vector<MemPtr> list;
};
