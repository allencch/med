#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <mutex>
#include <vector>
#include <string>
#include <mutex>
#include "med/MedTypes.hpp"
#include "med/ScanParser.hpp"
#include "med/ThreadManager.hpp"
#include "med/MedCommon.hpp"
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
                      const ScanParser::OpType& op,
                      int lastDigit = -1);
  vector<MemPtr> filter(const vector<MemPtr>& list,
                        Byte* value,
                        int size,
                        const string& scanType,
                        const ScanParser::OpType& op);
  vector<MemPtr> filterUnknown(const vector<MemPtr>& list,
                               const string& scanType,
                               const ScanParser::OpType& op);
  vector<MemPtr> filterUnknownWithList(const vector<MemPtr>& list,
                                       const string& scanType,
                                       const ScanParser::OpType& op);
  vector<MemPtr>& saveSnapshot(const vector<MemPtr>& baseList);
  vector<MemPtr> filterSnapshot(const string& scanType, const ScanParser::OpType& op);

  vector<MemPtr> scanInner(Byte* value,
                           int size,
                           Address base,
                           int blockSize,
                           const string& scanType,
                           const ScanParser::OpType& op);
  vector<MemPtr> scanUnknownInner(Address base,
                                  int blockSize,
                                  const string& scanType);
  vector<MemPtr> filterInner(const vector<MemPtr>& list,
                             Byte* value,
                             int size,
                             const string& scanType,
                             const ScanParser::OpType& op);
  vector<MemPtr> filterUnknownInner(const vector<MemPtr>& list,
                                    const string& scanType,
                                    const ScanParser::OpType& op);

  AddressPair* getScope();
  void setScopeStart(Address addr);
  void setScopeEnd(Address addr);

  std::mutex& getListMutex();

private:
  void initialize();
  Maps getInterestedMaps(Maps& maps, const vector<MemPtr>& list);
  void compareBlocks(vector<MemPtr>& list, MemPtr& oldBlock, MemPtr& newBlock, const string& scanType, const ScanParser::OpType& op);

  vector<MemPtr> scanByScope(Byte* value,
                             int size,
                             const string& scanType,
                             const ScanParser::OpType& op,
                             int lastDigit = -1);
  vector<MemPtr> scanByMaps(Byte* value,
                            int size,
                            const string& scanType,
                            const ScanParser::OpType& op,
                            int lastDigit = -1);

  static void scanMap(MemIO* memio,
                      std::mutex& mutex,
                      vector<MemPtr>& list,
                      Maps& maps,
                      int mapIndex,
                      int fd,
                      Byte* data,
                      int size,
                      const string& scanType,
                      const ScanParser::OpType& op,
                      int lastDigit = -1);

  vector<MemPtr>& saveSnapshotByScope();
  vector<MemPtr>& saveSnapshotByList(const vector<MemPtr>& baseList);

  static void saveSnapshotMap(MemIO* memio,
                              vector<MemPtr>& snapshot,
                              Maps& maps,
                              int mapIndex);
  static void scanPage(MemIO* memio,
                       std::mutex& mutex,
                       vector<MemPtr>& list,
                       Byte* page,
                       Address start,
                       Byte* value,
                       int size,
                       const string& scanType,
                       const ScanParser::OpType& op,
                       int lastDigit = -1);

  static void filterByChunk(std::mutex& mutex,
                            const vector<MemPtr>& list,
                            vector<MemPtr>& newList,
                            int listIndex,
                            Byte* value,
                            int size,
                            const string& scanType,
                            const ScanParser::OpType& op);
  static void filterUnknownByChunk(std::mutex& mutex,
                                   const vector<MemPtr>& list,
                                   vector<MemPtr>& newList,
                                   int listIndex,
                                   const string& scanType,
                                   const ScanParser::OpType& op);

  bool hasScope();

  pid_t pid;
  ThreadManager* threadManager;
  MemIO* memio;
  vector<MemPtr> snapshot;
  AddressPair* scope;
  std::mutex listMutex;
};

#endif
