#ifndef MEM_ED_HPP
#define MEM_ED_HPP

#include "mem/MemScanner.hpp"
#include "mem/MemManager.hpp"
#include "mem/MemList.hpp"
#include "med/Process.hpp"

class MemEd {
public:
  MemEd();
  explicit MemEd(pid_t pid);
  ~MemEd();
  void setPid(pid_t pid);
  pid_t getPid();
  vector<MemPtr> scan(const string& value, const string& scanType);
  vector<MemPtr> filter(const string& value, const string& scanType);
  MemList getScans();
  void clearScans();

  vector<Process> listProcesses();
  Process selectProcessByIndex(int index);
  vector<Process> processes;
  Process selectedProcess;
  
private:
  void initialize();
  pid_t pid;
  MemScanner* scanner;
  MemManager* manager;
};

#endif
