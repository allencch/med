#include <iostream>
#include <unistd.h> //getpagesize()

#include "mem/MemScanner.hpp"
#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"
#include "mem/Pem.hpp"
#include "mem/MemList.hpp"

using namespace std;

const int STEP = 1;
const int CHUNK_SIZE = 128;

MemScanner::MemScanner() {
  pid = 0;
  initialize();
}

MemScanner::MemScanner(pid_t pid) {
  this->pid = pid;
  initialize();
  memio->setPid(pid);
}

MemScanner::~MemScanner() {
  pid = 0;
  delete memio;
  delete threadManager;
}

void MemScanner::initialize() {
  threadManager = new ThreadManager();
  threadManager->setMaxThreads(8);
  memio = new MemIO();
}

void MemScanner::setPid(pid_t pid) {
  this->pid = pid;
  memio->setPid(pid);
}

pid_t MemScanner::getPid() {
  return pid;
}

MemIO* MemScanner::getMemIO() {
  return memio;
}

vector<MemPtr> MemScanner::scanInner(Byte* value,
                                     int size,
                                     Address base,
                                     int blockSize,
                                     const string& scanType,
                                     const ScanParser::OpType& op) {
  vector<MemPtr> list;
  for (Address addr = base; addr + size <= base + blockSize; addr += STEP) {
    if (memCompare((void*)addr, size, value, size, op)) {
      MemPtr mem = memio->read(addr, size);
      PemPtr pem = Pem::convertToPemPtr(mem, memio);
      pem->setScanType(scanType);

      list.push_back(pem);
    }
  }
  return list;
}

vector<MemPtr> MemScanner::filterInner(const vector<MemPtr>& list,
                                       Byte* value,
                                       int size,
                                       const string& scanType,
                                       const ScanParser::OpType& op) {
  vector<MemPtr> newList;
  for (size_t i = 0; i < list.size(); i++) {
    MemPtr mem = memio->read(list[i]->getAddress(), list[i]->getSize());

    if (memCompare(mem->getData(), size, value, size, op)) {
      PemPtr pem = Pem::convertToPemPtr(mem, memio);
      pem->setScanType(scanType);
      newList.push_back(pem);
    }
  }
  return newList;
}

vector<MemPtr> MemScanner::scan(Byte* value,
                                int size,
                                const string& scanType,
                                const ScanParser::OpType& op) {
  vector<MemPtr> list;

  ProcMaps maps = getMaps(pid);
  int memFd = getMem(pid);
  MemIO* memio = getMemIO();

  auto& mutex = listMutex;

  // TODO: Scan for unknown value

  for (size_t i = 0; i < maps.starts.size(); i++) {
    TMTask* fn = new TMTask();
    *fn = [memio, &mutex, &list, &maps, i, memFd, value, size, scanType, op]() {
      scanMap(memio, mutex, list, maps, i, memFd, value, size, scanType, op);
    };
    threadManager->queueTask(fn);
  }
  threadManager->start();
  threadManager->clear();

  return MemList::sortByAddress(list);
}

void MemScanner::scanMap(MemIO* memio,
                         std::mutex& mutex,
                         vector<MemPtr>& list,
                         ProcMaps& maps,
                         int mapIndex,
                         int fd,
                         Byte* value,
                         int size,
                         const string& scanType,
                         const ScanParser::OpType& op) {
  for (Address j = maps.starts[mapIndex]; j < maps.ends[mapIndex]; j += getpagesize()) {
      if(lseek(fd, j, SEEK_SET) == -1) {
      continue;
    }

    Byte* page = new Byte[getpagesize()]; //For block of memory

    if (read(fd, page, getpagesize()) == -1) {
      continue;
    }
    scanPage(memio, mutex, list, page, j, value, size, scanType, op);

    delete[] page;
  }
}

void MemScanner::scanPage(MemIO* memio,
                          std::mutex& mutex,
                          vector<MemPtr>& list,
                          Byte* page,
                          Address start,
                          Byte* value,
                          int size,
                          const string& scanType,
                          const ScanParser::OpType& op) {
  for (int k = 0; k <= getpagesize() - size; k += STEP) {
    try {
      if (memCompare(page + k, size, value, size, op)) {
        MemPtr mem = memio->read((Address)(start + k), size);

        PemPtr pem = Pem::convertToPemPtr(mem, memio);
        pem->setScanType(scanType);

        mutex.lock();
        list.push_back(pem);
        mutex.unlock();
      }
    } catch(MedException& ex) {
      cerr << ex.getMessage() << endl;
    }
  }
}

vector<MemPtr> MemScanner::filter(const vector<MemPtr>& list,
                                  Byte* value,
                                  int size,
                                  const string& scanType,
                                  const ScanParser::OpType& op) {
  vector<MemPtr> newList;
  int memFd = getMem(pid);

  auto& mutex = listMutex;

  for (size_t i = 0; i < list.size(); i += CHUNK_SIZE) {
    TMTask* fn = new TMTask();
    *fn = [&mutex, &list, &newList, i, memFd, value, size, scanType, op]() {
      filterByChunk(mutex, list, newList, i, memFd, value, size, scanType, op);
    };
    threadManager->queueTask(fn);
  }
  threadManager->start();
  threadManager->clear();

  return MemList::sortByAddress(newList);
}

void MemScanner::filterByChunk(std::mutex& mutex,
                               const vector<MemPtr>& list,
                               vector<MemPtr>& newList,
                               int listIndex,
                               int fd,
                               Byte* value,
                               int size,
                               const string& scanType,
                               const ScanParser::OpType& op) {
  Byte* buf = new Byte[size];
  for (int i = listIndex; i < listIndex + CHUNK_SIZE && i < (int)list.size(); i++) {
    if (lseek(fd, list[i]->getAddress(), SEEK_SET) == -1) {
      continue;
    }
    if (read(fd, buf, size) == -1) {
      continue;
    }

    if (memCompare(buf, size, value, size, op)) {
      PemPtr pem = static_pointer_cast<Pem>(list[i]);
      pem->setScanType(scanType);

      mutex.lock();
      newList.push_back(pem);
      mutex.unlock();
    }
  }
  delete[] buf;
}
