#ifndef MEM_ED_HPP
#define MEM_ED_HPP

#include "mem/MemScanner.hpp"
#include "mem/MemManager.hpp"

class MemEd {
public:
  MemEd();
  explicit MemEd(pid_t pid);
  ~MemEd();
  void setPid(pid_t pid);
  pid_t getPid();
  vector<MemPtr> scan(const string& value);
  vector<MemPtr> filter(const string& value);
  vector<MemPtr>& getMems();
  
private:
  void initialize();
  pid_t pid;
  MemScanner* scanner;
  MemManager* manager;
};

#endif
