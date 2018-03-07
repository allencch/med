#include <cstdio>
#include <string>

#include "med/MedCommon.hpp"
#include "mem/MemIO.hpp"

/**
 * Arguments: {pid} {address} {value}
 */
int main(int argc, char** argv) {
  MemIO* memIO = new MemIO();
  if (argc < 4) {
    printf("Missing arguments\n");
    return -1;
  }
  pid_t pid = stol(string(argv[1]));
  Address addr = hexToInt(string(argv[2]));
  int value = stoi(string(argv[3]));
  memIO->setPid(pid);
  MemPtr mem = memIO->read(addr, 4);
  mem->dump();

  return 0;
}
