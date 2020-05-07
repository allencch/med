#include <cerrno>  //errno
#include <cstring> //strerror()
#include <cinttypes>
#include <string>
#include <iostream>

#include <sys/ptrace.h> //ptrace()
#include <sys/prctl.h> //prctl()
#include <unistd.h> //getpagesize()
#include <fcntl.h> //open, read, lseek

#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"

using namespace std;

/**
 * Dump the hexa deximal
 */
void memDump(pid_t pid,Address address,int size) {
  pidAttach(pid);
  printf("%d\n",size);
  int memFd = getMem(pid);
  Byte* buf = new Byte[size];

  if(lseek(memFd, address, SEEK_SET) == -1) {
    printf("lseek error: %p, %s\n",(void*)address,strerror(errno));
    //continue;
  }

  if(read(memFd, buf, size) == -1) {
    printf("read error: %p, %s\n",(void*)address,strerror(errno));
    //continue;
  }

  for(int i=0;i<size;i++) {
    printf("%02x ",buf[i]);
  }
  printf("\n");
  delete[] buf;
  close(memFd);
  pidDetach(pid);
}

void memDirectDump(Byte* byteStart, int size) {
  Byte* buf = new Byte[size];
  memcpy(buf, (void*)byteStart, size);
  for(int i = 0; i < size ; i++) {
    printf("%x ", buf[i]);
  }
  printf("\n");
  delete[] buf;
}


/**
 * Reverse the memory (Big to Little Endian or vice versa)
 */
void memReverse(uint8_t* buf,int size) {
  Byte* temp = new Byte[size];
  memcpy(temp,buf,size);

  for(int i=0;i<size;i++) {
    buf[size-i-1] = temp[i];
  }

  delete[] temp;
}


bool memEq(const void* ptr1, const void* ptr2, size_t size) {
  int ret = memcmp(ptr1, ptr2, size);
  if (ret == 0)
    return true;
  return false;
}

bool memGt(const void* ptr1, const void* ptr2, size_t size) {
  //This is tricky, because of the little endianness, memcmp should check from right to left, not from left to right
  // Thus, need to reverse the memory

  Byte* rev1 = new Byte[size];
  Byte* rev2 = new Byte[size];
  memcpy(rev1, ptr1, size);
  memcpy(rev2, ptr2, size);

  memReverse(rev1, size);
  memReverse(rev2, size);

  int ret = memcmp(rev1, rev2, size);

  delete[] rev1;
  delete[] rev2;

  if (ret > 0) return true;

  return false;
}

bool memLt(const void* ptr1, const void* ptr2, size_t size) {
  Byte* rev1 = new Byte[size];
  Byte* rev2 = new Byte[size];
  memcpy(rev1, ptr1, size);
  memcpy(rev2, ptr2, size);

  memReverse(rev1, size);
  memReverse(rev2, size);

  int ret = memcmp(rev1, rev2, size);

  delete[] rev1;
  delete[] rev2;

  if (ret < 0) return true;

  return false;
}

bool memNeq(const void* ptr1, const void* ptr2, size_t size) {
  return !memEq(ptr1, ptr2, size);
}
bool memGe(const void* ptr1, const void* ptr2, size_t size) {
  return memGt(ptr1, ptr2, size) || memEq(ptr1, ptr2, size);
}
bool memLe(const void* ptr1, const void* ptr2, size_t size) {
  return memLt(ptr1, ptr2, size) || memEq(ptr1, ptr2, size);
}

bool memCompare(const void* ptr1, const void* ptr2, size_t size, ScanParser::OpType op) {
  if (op == ScanParser::Gt)
    return memGt(ptr1, ptr2, size);
  else if (op == ScanParser::Lt)
    return memLt(ptr1, ptr2, size);
  else if (op == ScanParser::Ge)
    return memGe(ptr1, ptr2, size);
  else if (op == ScanParser::Le)
    return memLe(ptr1, ptr2, size);
  else if (op == ScanParser::Neq)
    return memNeq(ptr1, ptr2, size);
  else
    return memEq(ptr1, ptr2, size);
}

// @deprecated
bool memCompare(const void* ptr1, size_t size1, const void* ptr2, size_t size2, ScanParser::OpType op) {
  if (op != ScanParser::Within) {
    return memCompare(ptr1, ptr2, size1, op);
  }

  int chunk = size2 / size1; //TODO: will be used for array
  if (chunk < 2) {
    throw MedException("Scan value is not array");
  }
  return memWithin(ptr1, ptr2, (uint8_t*)ptr2 + size1, size1);
}

bool memCompare(const void* ptr, size_t size, Operands& operands, const ScanParser::OpType& op) {
  SizedBytes firstOperand = operands.getFirstOperand();
  if (op != ScanParser::Within) {
    return memCompare(ptr, firstOperand.getBytes(), size, op);
  }
  SizedBytes secondOperand = operands.getSecondOperand();
  return memWithin(ptr, firstOperand.getBytes(), secondOperand.getBytes(), size);
}

bool memWithin(const void* src, const void* low, const void* high, size_t size) {
  return memGe(src, low, size) && memLe(src, high, size);
}

string memToString(Byte* memory, string scanType) {
  char str[MAX_STRING_SIZE];
  switch (stringToScanType(scanType)) {
  case Int8:
    sprintf(str, "%" PRIu8, *(uint8_t*)memory);
    break;
  case Int16:
    sprintf(str, "%" PRIu16, *(uint16_t*)memory);
    break;
  case Int32:
    sprintf(str, "%" PRIu32, *(uint32_t*)memory);
    break;
  case Ptr32:
    sprintf(str, "0x%" PRIx32, *(uint32_t*)memory);
    break;
  case Ptr64:
    sprintf(str, "0x%" PRIx64, *(uint64_t*)memory);
    break;
  case Float32:
    sprintf(str, "%f", *(float*)memory);
    break;
  case Float64:
    sprintf(str, "%lf", *(double*)memory);
    break;
  case String:
    sprintf(str, "%s", memory);
    break;
  case Custom:
    printf("Custom not able directly write to string\n");
    break;
  case Unknown:
    throw MedException("memToString: Error Type");
  }
  return string(str);
}

Address addressRoundDown(Address addr) {
  return addr - (addr % 16);
}
