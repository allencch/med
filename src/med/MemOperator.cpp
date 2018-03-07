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
void memDump(pid_t pid,MemAddr address,int size) {
  pidAttach(pid);
  printf("%d\n",size);
  int memFd = getMem(pid);
  uint8_t* buf = (uint8_t*)malloc(size);

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
  free(buf);
  close(memFd);
  pidDetach(pid);
}

void memDirectDump(Byte* byteStart, int size) {
  uint8_t* buf = (uint8_t*)malloc(size);
  memcpy(buf, (void*)byteStart, size);
  for(int i = 0; i < size ; i++) {
    printf("%x ", buf[i]);
  }
  printf("\n");
  free(buf);
}


/**
 * @param size is the size based on the byte
 */
void memWrite(pid_t pid, MemAddr address, uint8_t* data, int size) {
  medMutex.lock();
  pidAttach(pid);
  int psize = padWordSize(size); //padded size

  uint8_t* buf = (uint8_t*)malloc(psize);
  long word;
  for(int i=0;i<psize;i+=sizeof(long)) {
    errno = 0;
    word = ptrace(PTRACE_PEEKDATA,pid,(uint8_t*)(address) + i, NULL);

    if(errno) {
      printf("PEEKDATA error: %p, %s\n", (void*)address, strerror(errno));
    }

    //Write word to the buffer
    memcpy((uint8_t*)buf + i, &word, sizeof(long));
  }

  memcpy(buf,data,size); //over-write on top of it, so that the last padding byte will preserved

  for(int i=0;i<size;i+=sizeof(long)) {
    // This writes as uint32, it should be uint8
    // According to manual, it reads "word". Depend on the CPU.
    // If the OS is 32bit, then word is 32bit; if 64bit, then 64bit.
    // Thus, the best solution is peek first, then only over write the position
    // Therefore, the value should be the WORD size.

    if(ptrace(PTRACE_POKEDATA,pid,(uint8_t*)(address)+i,*(long*)((uint8_t*)buf+i) ) == -1L) {
      printf("POKEDATA error: %s\n",strerror(errno));
    }
  }

  free(buf);
  pidDetach(pid);

  medMutex.unlock();
}


void memWriteList(Scanner scanner,pid_t pid,uint8_t* data,int size) {
  pidAttach(pid);
  uint8_t* buf = (uint8_t*)malloc(size+sizeof(long)); //plus 8 bytes, to avoid WORD problem, seems like "long" can represent "word"

  for(unsigned int j=0;j<scanner.addresses.size();j++) {

    long word;
    for(int i=0;i<size;i+=sizeof(long)) {
      errno = 0;
      word = ptrace(PTRACE_PEEKDATA,pid,scanner.addresses[j]+i,NULL);
      if(errno) {
        printf("PEEKDATA error: %p, %s\n",(void*)(scanner.addresses[j]+i),strerror(errno));
        //exit(1);
      }

      //Write word to the buffer
      memcpy((uint8_t*)buf+i,&word,sizeof(long));
    }

    memcpy(buf,data,size);


    for(int i=0;i<size;i+=sizeof(long)) {
      if(ptrace(PTRACE_POKEDATA,pid,scanner.addresses[j]+i,*(long*)((uint8_t*)data+i) ) == -1L) {
        printf("POKEDATA error: %s\n",strerror(errno));
        //exit(1);
      }
    }
  }

  free(buf);
  printf("Success!\n");
  pidDetach(pid);
  //memDump(pid,address,10);
}

/**
 * Reverse the memory (Big to Little Endian or vice versa)
 */
void memReverse(uint8_t* buf,int size) {
  uint8_t* temp = (uint8_t*)malloc(size);
  memcpy(temp,buf,size);

  for(int i=0;i<size;i++) {
    buf[size-i-1] = temp[i];
  }

  free(temp);
}


string memValue(long pid, MemAddr address, string scanType) {
  medMutex.lock();
  pidAttach(pid);

  int size = scanTypeToSize(stringToScanType(scanType));

  int memFd = getMem(pid);
  uint8_t* buf = (uint8_t*)malloc(size + 1); //+1 for the NULL
  memset(buf, 0, size + 1);

  if(lseek(memFd, address, SEEK_SET) == -1) {
    free(buf);
    close(memFd);
    pidDetach(pid);
    medMutex.unlock();
    throw MedException("Address seek fail");
  }
  if(read(memFd, buf, size) == -1) {
    free(buf);
    close(memFd);
    pidDetach(pid);
    medMutex.unlock();
    throw MedException("Address read fail: " + intToHex(address));
  }

  char str[MAX_STRING_SIZE + 1];
  switch (stringToScanType(scanType)) {
  case Int8:
    sprintf(str, "%" PRIu8, *(uint8_t*)buf);
    break;
  case Int16:
    sprintf(str, "%" PRIu16, *(uint16_t*)buf);
    break;
  case Int32:
    sprintf(str, "%" PRIu32, *(uint32_t*)buf);
    break;
  case Float32:
    sprintf(str, "%f", *(float*)buf);
    break;
  case Float64:
    sprintf(str, "%lf", *(double*)buf);
    break;
  case String:
    sprintf(str, "%s", buf);
    break;
  case Unknown:
    free(buf);
    close(memFd);
    pidDetach(pid);
    medMutex.unlock();
    throw MedException(string("memValue: Error Type: ") + scanType);
  }

  free(buf);
  close(memFd);
  pidDetach(pid);
  medMutex.unlock();

  string ret = string(str);
  return ret;
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

  uint8_t* rev1 = (uint8_t*)malloc(size);
  uint8_t* rev2 = (uint8_t*)malloc(size);
  memcpy(rev1, ptr1, size);
  memcpy(rev2, ptr2, size);

  memReverse(rev1, size);
  memReverse(rev2, size);

  int ret = memcmp(rev1, rev2, size);

  free(rev1);
  free(rev2);

  if (ret > 0)
    return true;
  return false;
}

bool memLt(const void* ptr1, const void* ptr2, size_t size) {
  uint8_t* rev1 = (uint8_t*)malloc(size);
  uint8_t* rev2 = (uint8_t*)malloc(size);
  memcpy(rev1, ptr1, size);
  memcpy(rev2, ptr2, size);

  memReverse(rev1, size);
  memReverse(rev2, size);

  int ret = memcmp(rev1, rev2, size);

  free(rev1);
  free(rev2);

  if (ret < 0)
    return true;
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

bool memCompare(const void* ptr1, size_t size1, const void* ptr2, size_t size2, ScanParser::OpType op) {
  if (op != ScanParser::Within)
    return memCompare(ptr1, ptr2, size1, op);

  int chunk = size2 / size1; //TODO: will be used for array
  if (chunk < 2) {
    throw MedException("Scan value is not array");
  }
  return memWithin(ptr1, ptr2, (uint8_t*)ptr2 + size1, size1);
}

bool memWithin(const void* src, const void* low, const void* up, size_t size) {
  return memGe(src, low, size) && memLe(src, up, size);
}

MemAddr addressRoundDown(MemAddr addr) {
  return addr - (addr % 16);
}


Byte* memRead(pid_t pid, MemAddr address, size_t size) {
  medMutex.lock();
  pidAttach(pid);

  int memFd = getMem(pid);
  Byte* buf = (Byte*)malloc(size + 1);
  memset(buf, 0, size + 1);

  if(lseek(memFd, address, SEEK_SET) == -1) {
    free(buf);
    close(memFd);
    pidDetach(pid);
    throw MedException("Failed lseek");
  }
  else if(read(memFd, buf, size) == -1) {
    free(buf);
    close(memFd);
    pidDetach(pid);
    medMutex.unlock();
    throw MedException("Address read fail: " + intToHex(address));
  }

  close(memFd);
  pidDetach(pid);
  medMutex.unlock();
  return buf; // Remember to free this
}

string memToString(Byte* memory, string scanType) {
  char str[MAX_STRING_SIZE + 1];
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
  case Float32:
    sprintf(str, "%f", *(float*)memory);
    break;
  case Float64:
    sprintf(str, "%lf", *(double*)memory);
    break;
  case String:
    sprintf(str, "%s", memory);
    break;
  case Unknown:
    throw MedException("memToString: Error Type");
  }
  return string(str);
}
