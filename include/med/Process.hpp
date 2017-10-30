#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>

using namespace std;

class Process {
public:
  Process();
  string pid;
  string cmdline; //aka "process" in GUI
};

#endif
