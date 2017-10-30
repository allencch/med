#include "med/Process.hpp"
#include "med/MemOperator.hpp"

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

// Bytes* Process::pullMemory() {
//   mtx.lock();
//   long pid = stoi(this->pid);

//   pidAttach(pid);

//   ProcMaps maps = getMaps(pid);
//   int memFd = getMem(pid);

//   for (int i = 0; i < (int)maps.starts.size(); i++) {

//   }


//   pidDetach(pid);
//   mtx.unlock();
// }
