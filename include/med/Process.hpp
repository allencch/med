#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <mutex>

#include "med/MedTypes.hpp"
#include "med/Bytes.hpp"

using namespace std;

class Process {
public:
  Process();
  Process(const Process& process); // Required, because mutex is not copiable
  Process& operator=(const Process& process);

  string pid;
  string cmdline; //aka "process" in GUI

  //Bytes* pullMemory();
private:
  std::mutex mutex;
};

// class ProcessMemory {
// public:
//   ProcessMemory();
// };

#endif
