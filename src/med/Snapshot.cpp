#include <iostream>

#include "med/Process.hpp"
#include "med/Snapshot.hpp"
#include "med/MedException.hpp"

using namespace std;

Snapshot::Snapshot() {}
Snapshot::~Snapshot() {
  memoryBlocks.clear();
}

void Snapshot::save(Process* process) {
  if (process->pid.length() == 0) {
    MedException("Process ID is not empty");
    return;
  }
  memoryBlocks.clear();
  memoryBlocks = process->pullMemory();
}
