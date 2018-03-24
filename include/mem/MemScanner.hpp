#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <mutex>
#include <vector>
#include <string>
#include <mutex>
#include "med/MedTypes.hpp"
#include "med/ScanParser.hpp"
#include "med/ThreadManager.hpp"
#include "mem/Mem.hpp"
#include "mem/MemIO.hpp"

using namespace std;

class MemScanner {
public:
  MemScanner();
  explicit MemScanner(pid_t pid);
  ~MemScanner();
  void setPid(pid_t pid);
  pid_t getPid();
  MemIO* getMemIO();
  vector<MemPtr> scan(Byte* value,
                      int size,
                      const string& scanType,
                      const ScanParser::OpType& op);
  vector<MemPtr> filter(const vector<MemPtr>& list,
                        Byte* value,
                        int size,
                        const string& scanType,
                        const ScanParser::OpType& op);

  vector<MemPtr> scanInner(Byte* value,
                           int size,
                           Address base,
                           int blockSize,
                           const string& scanType,
                           const ScanParser::OpType& op);
  vector<MemPtr> filterInner(const vector<MemPtr>& list,
                             Byte* value,
                             int size,
                             const string& scanType,
                             const ScanParser::OpType& op);
  vector<MemPtr> filterUnknownInner(const vector<MemPtr>& list,
                                    const string& scanType,
                                    const ScanParser::OpType& op);
  std::mutex listMutex;

private:
  void initialize();
  static void scanMap(MemIO* memio,
                      std::mutex& mutex,
                      vector<MemPtr>& list,
                      ProcMaps& maps,
                      int mapIndex,
                      int fd,
                      Byte* data,
                      int size,
                      const string& scanType,
                      const ScanParser::OpType& op);
  static void scanPage(MemIO* memio,
                       std::mutex& mutex,
                       vector<MemPtr>& list,
                       Byte* page,
                       Address start,
                       Byte* value,
                       int size,
                       const string& scanType,
                       const ScanParser::OpType& op);
  static void filterByChunk(std::mutex& mutex,
                            const vector<MemPtr>& list,
                            vector<MemPtr>& newList,
                            int listIndex,
                            Byte* value,
                            int size,
                            const string& scanType,
                            const ScanParser::OpType& op);
  pid_t pid;
  ThreadManager* threadManager;
  MemIO* memio;
};

#endif
