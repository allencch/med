#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <mutex>
#include <vector>
#include <string>
#include <mutex>
#include "med/MedTypes.hpp"
#include "med/ScanParser.hpp"
#include "med/ThreadManager.hpp"
#include "med/Operands.hpp"
#include "med/MedCommon.hpp"
#include "med/ScanCommand.hpp"
#include "mem/Mem.hpp"
#include "mem/MemIO.hpp"
#include "mem/ScanParams.hpp"

using namespace std;

class MemScanner {
public:
  MemScanner();
  explicit MemScanner(pid_t pid);
  ~MemScanner();
  void setPid(pid_t pid);
  pid_t getPid();
  MemIO* getMemIO();
  vector<MemPtr> scan(Operands& operands,
                      int size,
                      const string& scanType,
                      const ScanParser::OpType& op,
                      bool fastScan = false,
                      int lastDigit = -1);
  vector<MemPtr> scan(ScanCommand &scanCommand);
  vector<MemPtr> filter(const vector<MemPtr>& list,
                        Operands& operands,
                        int size,
                        const string& scanType,
                        const ScanParser::OpType& op);
  vector<MemPtr> filter(const vector<MemPtr> &list, ScanCommand &scanCommand);
  vector<MemPtr> filterUnknown(const vector<MemPtr>& list,
                               const string& scanType,
                               const ScanParser::OpType& op,
                               bool fastScan = false);
  vector<MemPtr> filterUnknownWithList(const vector<MemPtr>& list,
                                       const string& scanType,
                                       const ScanParser::OpType& op);
  vector<MemPtr>& saveSnapshot(const vector<MemPtr>& baseList);
  vector<MemPtr> filterSnapshot(const string& scanType, const ScanParser::OpType& op, bool fastScan = false);

  vector<MemPtr> scanInner(Operands& operands,
                           int size,
                           Address base,
                           int blockSize,
                           const string& scanType,
                           const ScanParser::OpType& op);
  vector<MemPtr> scanUnknownInner(Address base,
                                  int blockSize,
                                  const string& scanType);
  vector<MemPtr> filterInner(const vector<MemPtr>& list,
                             Operands& operands,
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
  void compareBlocks(vector<MemPtr>& list,
                     MemPtr& oldBlock,
                     MemPtr& newBlock,
                     const string& scanType,
                     const ScanParser::OpType& op,
                     bool fastScan = false);

  vector<MemPtr> scanByScope(ScanCommand &scanCommand);

  vector<MemPtr> scanByMaps(Operands& operands,
                            int size,
                            const string& scanType,
                            const ScanParser::OpType& op,
                            bool fastScan = false,
                            int lastDigit = -1);
  vector<MemPtr> scanByMaps(ScanCommand &scanCommand);

  static void scanMap(ScanParams params);
  static void scanMap(MemIO* memio,
                      std::mutex& mutex,
                      vector<MemPtr>& list,
                      Maps& maps,
                      int mapIndex,
                      int fd,
                      std::mutex& fdMutex,
                      ScanCommand &scanCommand);

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
                       Operands& operands,
                       int size,
                       const string& scanType,
                       const ScanParser::OpType& op,
                       bool fastScan = false,
                       int lastDigit = -1);
  static void scanPage(MemIO* memio,
                       std::mutex& mutex,
                       vector<MemPtr>& list,
                       Byte* page,
                       Address start,
                       ScanCommand &scanCommand);

  static void filterByChunk(std::mutex& mutex,
                            const vector<MemPtr>& list,
                            vector<MemPtr>& newList,
                            int listIndex,
                            Operands& operands,
                            int size,
                            const string& scanType,
                            const ScanParser::OpType& op);
  static void filterByChunk(std::mutex& mutex,
                            const vector<MemPtr>& list,
                            vector<MemPtr>& newList,
                            int listIndex,
                            ScanCommand &scanCommand);
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
