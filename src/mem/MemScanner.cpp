#include <iostream>
#include <unistd.h> //getpagesize()

#include "mem/MemScanner.hpp"
#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"

using namespace std;

const int STEP = 1;

MemScanner::MemScanner() {
  pid = 0;
  threadManager = &ThreadManager::getInstance();
  threadManager->setMaxThreads(8);
}

MemScanner::MemScanner(pid_t pid) {
  this->pid = pid;
}

void MemScanner::setPid(pid_t pid) {
  this->pid = pid;
}

pid_t MemScanner::getPid() {
  return pid;
}

vector<MemPtr> MemScanner::scan(Byte* value, int size, string scanType, ScanParser::OpType op) {
  vector<MemPtr> list;
  mutex.lock();
  pidAttach(pid);

  ProcMaps maps = getMaps(pid);
  int memFd = getMem(pid);

  for (size_t i = 0; i < maps.starts.size(); i++) {
    TMTask* fn = new TMTask();
    *fn = [&list, &maps, i, memFd, value, size, scanType, op]() {
      scanMap(list, maps, i, memFd, value, size, scanType, op);
    };
    threadManager->queueTask(fn);
  }
  return list;
}

void MemScanner::scanMap(vector<MemPtr>& list,
                      ProcMaps& maps,
                      int mapIndex,
                      int fd,
                      Byte* value,
                      int size,
                      string scanType,
                      ScanParser::OpType op) {
  for (MemAddr j = maps.starts[mapIndex]; j < maps.ends[mapIndex]; j += getpagesize()) {
      if(lseek(fd, j, SEEK_SET) == -1) {
      continue;
    }

    Byte* page = (Byte*)malloc(getpagesize()); //For block of memory

    if(read(fd, page, getpagesize()) == -1) {
      continue;
    }
    scanPage(list, page, j, value, size, scanType, op);

    free(page);
  }
}

void MemScanner::scanPage(vector<MemPtr>& list,
                       Byte* page,
                       Address start,
                       Byte* value,
                       int size,
                       string scanType, // not using it right now
                       ScanParser::OpType op) {
  for(int k = 0; k <= getpagesize() - size; k += STEP) {
    try {
      if(memCompare(page + k, size, value, size, op)) {
        MemPtr mem = MemPtr(new Mem(start + k, size));
        list.push_back(mem);
      }
    } catch(MedException& ex) {
      cerr << ex.getMessage() << endl;
    }
  }
}
