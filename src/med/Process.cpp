#include <unistd.h> //getpagesize()

#include "med/Process.hpp"
#include "med/MemOperator.hpp"
#include "med/ByteManager.hpp"

Process::Process() {}

Process::Process(const Process& process) {
  pid = process.pid;
  cmdline = process.cmdline;
}

Process& Process::operator=(const Process& process) {
  this->pid = process.pid;
  this->cmdline = process.cmdline;
  return *this;
}

MemoryBlocks Process::pullMemory() {
  mtx.lock();
  long pid = stoi(this->pid);

  pidAttach(pid);

  ProcMaps maps = getMaps(pid);
  int memFd = getMem(pid);

  MemoryBlocks blocks;
  for (int i = 0; i < (int)maps.starts.size(); i++) {
    pullMemoryByMap(maps, i, memFd, blocks);
  }

  pidDetach(pid);
  mtx.unlock();
  return blocks;
}

void Process::pullMemoryByMap(const ProcMaps& maps, int mapIndex, int memFd, MemoryBlocks& blocks) {
  int totalSize = maps.ends[mapIndex] - maps.starts[mapIndex];

  ByteManager& bm = ByteManager::getInstance();
  Byte* byte = bm.newByte(totalSize);

  MemoryBlock block;

  int pageSize = getpagesize();
  for (MemAddr j = maps.starts[mapIndex]; j < maps.ends[mapIndex]; j += pageSize) {
    Byte* pointer = byte + j;
    if (read(memFd, pointer, pageSize == -1) == -1) {
      continue;
    }
  }
  block.setDataWithAddress(byte, totalSize, maps.starts[mapIndex]);
  blocks.push(block);
}
