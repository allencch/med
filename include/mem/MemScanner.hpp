#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <mutex>
#include <vector>
#include <string>
#include "med/MedTypes.hpp"
#include "med/ScanParser.hpp"
#include "mem/Mem.hpp"
#include "med/ThreadManager.hpp"

using namespace std;

class MemScanner {
public:
  MemScanner();
  explicit MemScanner(pid_t pid);
  ~MemScanner();
  void setPid(pid_t pid);
  pid_t getPid();
  vector<MemPtr> scan(Byte* value, int size, string scanType, ScanParser::OpType op);

private:
  static void scanMap(vector<MemPtr>& list, ProcMaps& maps, int mapIndex, int fd, Byte* data, int size, string scanType, ScanParser::OpType op);
  static void scanPage(vector<MemPtr>& list, Byte* page, Address start, Byte* value, int size, string scanType, ScanParser::OpType op);
  pid_t pid;
  std::mutex mutex;
  ThreadManager* threadManager;
};

#endif
