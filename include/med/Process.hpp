#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <vector>

#include "med/MedTypes.hpp"

using namespace std;

class Process {
public:
  Process() {}

  string pid;
  string cmdline; //aka "process" in GUI
};

#endif
