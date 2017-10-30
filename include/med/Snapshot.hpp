#ifndef SNAPSHOT_HPP
#define SNAPSHOT_HPP

#include <string>

class Process;

class Snapshot {
public:
  Snapshot();
  void save(Process* process);
};

#endif
