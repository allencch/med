/**
 * @author	Allen Choong
 * @version	0.0.1
 * @date	2012-03-15
 *
 * Simple memory editor. The goal is to hack the Android game from adb shell
 * It is inspired by scanmem.
 * Actually intend to write in C, but I need list (vector), so I change
 * it to C++.
 * By default, all the search is little endian.
 *
 * Todo:
 * Scan integer, float, little endian, big endian, arrays
 * Edit integer, float, little endian, big endian, arrays
 * Dump with hex position
 * In the interactive mode,
 * try to use solve the keyboard problem, such as moving cursor.
 * Pause and play the process
 *
 *
 * Search by block
 *
 *
 * Fixed:
 * Solve the problem of the I/O Error for the large running file
 * On Android, the scan and edit does not work, because of permission limit.
 * Tried the signal.h kill() to SIGSTOP the process during scanning in Android, but still get the EIO
 * (I/O Error).
 *
 * Changelog:
 * 2015-05-20	Because of 64-bit computer, the address must use long int.
 *
 */


//#define _FILE_OFFSET_BITS 64


#include <cstdio>
#include <cstdlib>
#include <cstdint> //uint8

#include <cerrno>  //errno
#include <cstring> //strerror()
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <vector>
#include <cinttypes>
#include <chrono>
#include <algorithm>

#include <sys/ptrace.h> //ptrace()
#include <sys/wait.h> //waitpid()
#include <linux/capability.h>
#include <sys/prctl.h> //prctl()
#include <unistd.h> //getpagesize()
#include <fcntl.h> //open, read, lseek
#include <dirent.h> //read directory

#include <json/json.h>

#include "med.hpp"

using namespace std;

//Scanner scanner; //Global variable

/**
 * @brief Convert hexadecimal string to integer value
 */
long hexToInt(string str) throw(MedException) {
  stringstream ss(str);
  unsigned long ret = -1;
  ss >> hex >> ret;
  if(ss.fail()) {
    // fprintf(stderr,"Error input: %s\n",str.c_str());
    throw MedException(string("Error input: ") + str);
  }

  return ret;
}

/**
 * @brief Convert long integer to hex string
 */
string intToHex(long hex) {
  char str[64];
  sprintf(str,"%p",(void*)hex);
  return string(str);
}


ScanType stringToScanType(string scanType) {
  if(scanType == "int8") {
    return Int8;
  }
  else if(scanType == "int16") {
    return Int16;
  }
  else if(scanType == "int32") {
    return Int32;
  }
  else if(scanType == "float32") {
    return Float32;
  }
  else if(scanType == "float64") {
    return Float64;
  }
  return Unknown;
}

string scanTypeToString(ScanType scanType) {
  string ret = "unknown";
  switch(scanType) {
  case Int8:
    ret = "int8";
    break;
  case Int16:
    ret = "int16";
    break;
  case Int32:
    ret = "int32";
    break;
  case Float32:
    ret = "float32";
    break;
  case Float64:
    ret = "float64";
    break;
  }
  return ret;
}

int scanTypeToSize(ScanType type) {
  int ret = 0;
  switch(type) {
  case Int8:
    ret = sizeof(uint8_t);
    break;
  case Int16:
    ret = sizeof(uint16_t);
    break;
  case Int32:
    ret = sizeof(uint32_t);
    break;
  case Float32:
    ret = sizeof(float);
    break;
  case Float64:
    ret = sizeof(double);
    break;
  case Unknown:
    ret = 0;
  }
  return ret;
}



/**
 * Convert numerical string to raw data (hexadecimal).
 * @param s is a string which can separated by space
 * @param buffer is to store the output address, must be free() if not used.
 * @return number of values (bytes) to be scanned
 */
int stringToRaw(string str, ScanType type, uint8_t **buffer) {
  //Tokenise the string
  stringstream ss(str);
  istream_iterator<string> eos; //end of string
  istream_iterator<string> iit(ss);

  vector<string> tokens(iit,eos);

  int size = tokens.size();
  int retsize = 0;
  uint8_t* buf;

  switch(type) {
  case Int8:
    retsize = sizeof(uint8_t) * size;
    buf = (uint8_t*)malloc(sizeof(uint8_t) * size);
    break;
  case Int16:
    retsize = sizeof(uint16_t) * size;
    buf = (uint8_t*)malloc(sizeof(uint16_t) * size);
    break;
  case Int32:
    retsize = sizeof(uint32_t) * size;
    buf = (uint8_t*)malloc(sizeof(uint32_t) * size);
    break;
  case Float32:
    retsize = sizeof(float) * size;
    buf = (uint8_t*)malloc(sizeof(float) * size);
    break;
  case Float64:
    retsize = sizeof(double) * size;
    buf = (uint8_t*)malloc(sizeof(double) * size);
    break;
  case Unknown:
    retsize = 0;
    break;
  }

  for(unsigned int i=0;i<tokens.size();i++) {
    stringstream ss(tokens[i]);

    int temp;
    switch(type) {
    case Int8:
      ss >> dec >> temp; //Because it does not handle the uint8_t correctly as integer, but a character.
      *(uint8_t*)buf = (uint8_t) temp;
      break;
    case Int16:
      ss >> dec >> *(uint16_t*)buf;
      break;
    case Int32:
      ss >> dec >> *(uint32_t*)buf;
      break;
    case Float32:
      ss >> dec >> *(float*)buf;
      break;
    case Float64:
      ss >> dec >> *(double*)buf;
      break;
    case Unknown:
      break;
    }
    if(ss.fail()) {
      printf("Error input: %s\n",tokens[i].c_str());
    }
  }

  *buffer = buf;

  return retsize;
}


int stringToRaw(string str, string type, uint8_t **buffer) {
  ScanType scantype = stringToScanType(type);
  return stringToRaw(str,scantype,buffer);
}


/**
 * @brief Print the hexadecimal data
 */
void printHex(FILE* file,void* addr,int size) {
  for(int i=0;i<size;i++) {
    fprintf(file,"%02x ",((uint8_t*)addr)[i]);
  }
}

ProcMaps getMaps(pid_t pid) {
  ProcMaps procMaps;

  //Get the region from /proc/pid/maps
  char filename[128];
  sprintf(filename,"/proc/%d/maps",pid);
  FILE* file;
  file = fopen(filename,"r");
  if(!file) {
    printf("Failed open maps: %s\n",filename);
    exit(1);
  }


  char useless[64];
  unsigned long start,end;
  char rd,wr;
  int inode;
  //int ret;
  char sp; //shared or private
  char fname[128];

  //Get line
  char line[256];

  while(fgets(line,255,file)) {
    //parse line
    //the empty pathname has to be scan also
    if(sscanf(line,"%lx-%lx %c%c%c%c %s %s %u %s",
              &start,&end,
              &rd,&wr,useless,&sp,
              useless,useless,&inode,fname) <9) {
      continue;
    }

    if(rd == 'r' && wr == 'w' && ((end - start) > 0)) {
      procMaps.starts.push_back(start);
      procMaps.ends.push_back(end);
    }//*/
  }

  fclose(file);

  return procMaps;
}

/**
 * Open the /proc/[pid]/mem
 * @return file descriptor
 */
int getMem(pid_t pid) {
  char filename[32];
  sprintf(filename,"/proc/%d/mem",pid);
  //printf("filename: %s\n",filename);
  int ret = open(filename,O_RDONLY);
  if(ret == -1) {
    printf("Open failed: %s\n",strerror(errno));
  }
  return ret;
}


/**
 * Attach PID
 */
pid_t pidAttach(pid_t pid) throw(MedException) {
  //Attach the processr
  if(ptrace(PTRACE_ATTACH,pid,NULL,NULL) == -1L) {
    fprintf(stderr,"Failed attach: %s\n",strerror(errno));
    throw MedException("Failed attach");
  }

  int status;
  if(waitpid(pid,&status,0) == -1 || !WIFSTOPPED(status)) {
    fprintf(stderr,"Error waiting: %s\n",strerror(errno));
    throw MedException("Error waiting");
  }

  return pid;
}

pid_t pidDetach(pid_t pid) throw(MedException){
  //Detach
  if(ptrace(PTRACE_DETACH,pid,NULL,NULL) == -1L) {
    fprintf(stderr,"Failed detach: %s\n",strerror(errno));
    throw MedException("Failed detach");
  }
  return -1;
}


int memDump2(pid_t pid,unsigned long address,int size) {
  pidAttach(pid);

  long word;
  uint8_t* byteptr = (uint8_t*)(&word);
  for(int i=0;i<size;i+=sizeof(long)) {
    word = ptrace(PTRACE_PEEKDATA,pid,(void*)(address + i),NULL);
    if(errno) {
      printf("Error: %p, %s\n",(void*)address,strerror(errno));
      return 0;
    }
    for(unsigned int j=0;j<sizeof(long);j++) {
      printf("%02x ",*(byteptr+j));
    }
  }
  printf("\n");
  pidDetach(pid);
  return 1;
}

/**
 * Dump the hexa deximal
 */
int memDump(pid_t pid,unsigned long address,int size) {
  pidAttach(pid);
  printf("%d\n",size);
  int memFd = getMem(pid);
  uint8_t* buf = (uint8_t*)malloc(size);
  //for(int i=0;i<size;i++) {
  if(lseek(memFd,address,SEEK_SET) == -1) {
    printf("lseek error: %p, %s\n",(void*)address,strerror(errno));
    //continue;
  }

  if(read(memFd,buf,size) == -1) {
    printf("read error: %p, %s\n",(void*)address,strerror(errno));
    //continue;
  }
  //}


  for(int i=0;i<size;i++) {
    printf("%02x ",buf[i]);
  }
  printf("\n");
  free(buf);
  close(memFd);
  pidDetach(pid);
  return 1;
}

/**
 * Convert the size to padded word size.
 */
int padWordSize(int x) {
  if(x % sizeof(unsigned long))
    return x + sizeof(unsigned long) - x % sizeof(unsigned long);
  return x;
}

void memWrite2(pid_t pid,unsigned long address,uint8_t* data,int size) {
  pidAttach(pid);
  cout << "size: " <<size<< endl;
  uint8_t* buf = (uint8_t*)malloc(size+sizeof(long)); //plus 8 bytes, to avoid WORD problem, seems like "long" can represent "word"
  long word;
  for(int i=0;i<size;i+=sizeof(long)) {
    errno = 0;
    word = ptrace(PTRACE_PEEKDATA,pid,(uint8_t*)(address)+i,NULL);
    if(errno) {
      printf("PEEKDATA error: %p, %s\n",(void*)address,strerror(errno));
      //exit(1);
    }

    //Write word to the buffer
    memcpy((uint8_t*)buf+i,&word,sizeof(long));
  }


  memcpy(buf,data,size);


  for(int i=0;i<size;i+=sizeof(long)) {
    //FIXME: This writes as uint32, it should be uint8
    // According to manual, it read "word". Depend on the CPU.
    // If the OS is 32bit, then word is 32bit; if 64bit, then 64bit.
    // Thus, the best solution is peek first, then only over write the position

    // FIXED!!! The problem caused by the 4th parameter, which is the value, instead of address
    // Therefore, the value should be the WORD size.

    if(ptrace(PTRACE_POKEDATA,pid,(uint8_t*)(address)+i,*(long*)((uint8_t*)data+i) ) == -1L) {
      printf("POKEDATA error: %s\n",strerror(errno));
      //exit(1);
    }
  }


  free(buf);
  //printf("Success!\n");
  pidDetach(pid);
  memDump(pid,address,10); //Should I dump???
}

/**
 * @param size is the size based on the byte
 */
void memWrite(pid_t pid,unsigned long address,uint8_t* data,int size) {
  pidAttach(pid);
  int psize = padWordSize(size); //padded size

  uint8_t* buf = (uint8_t*)malloc(psize);
  long word;
  medMutex.lock();
  for(int i=0;i<psize;i+=sizeof(long)) {
    errno = 0;
    word = ptrace(PTRACE_PEEKDATA,pid,(uint8_t*)(address)+i,NULL);

    if(errno) {
      printf("PEEKDATA error: %p, %s\n",(void*)address,strerror(errno));
      //exit(1);
    }

    //Write word to the buffer
    memcpy((uint8_t*)buf+i,&word,sizeof(long));
  }

  memcpy(buf,data,size); //over-write on top of it, so that the last padding byte will preserved

  for(int i=0;i<size;i+=sizeof(long)) {
    //FIXME: This writes as uint32, it should be uint8
    // According to manual, it read "word". Depend on the CPU.
    // If the OS is 32bit, then word is 32bit; if 64bit, then 64bit.
    // Thus, the best solution is peek first, then only over write the position

    // FIXED!!! The problem caused by the 4th parameter, which is the value, instead of address
    // Therefore, the value should be the WORD size.

    if(ptrace(PTRACE_POKEDATA,pid,(uint8_t*)(address)+i,*(long*)((uint8_t*)buf+i) ) == -1L) {
      printf("POKEDATA error: %s\n",strerror(errno));
      //exit(1);
    }
  }


  free(buf);
  //printf("Success!\n");
  pidDetach(pid);
  //memDump(pid,address,10); //Should I dump???

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
 * Copy the memory from PEEKDATA
 * @deprecated because replaced by procfs mem method
 */
int memCopy(pid_t pid,unsigned long address,unsigned char* out,int size,bool showError=true) {
  for(int i=0;i<size;i++) {
    out[i] = ptrace(PTRACE_PEEKDATA,pid,(void*)(address + i),NULL);
    if(errno) {
      if(showError) {
        printf("Error: %p, %s\n",(void*)address,strerror(errno));
        //exit(1);
      }
      return 0;
    }
  }
  return 1;
}

/**
 * Scan memory, using procfs mem
 */
void memScanEqual(Scanner &scanner,pid_t pid,unsigned char* data,int size) {
  pidAttach(pid);
  scanner.addresses.clear();

  ProcMaps maps = getMaps(pid);


  uint8_t* page = (uint8_t*)malloc(getpagesize()); //For block of memory

  int memFd = getMem(pid);

  //Loop through maps
  for(unsigned int i=0;i<maps.starts.size();i++) {
    //printf("maps: %p\t%p\n", maps.starts[i],maps.ends[i]);
    //Loop each maps to get the "page"
    for(unsigned long int j=maps.starts[i];j<maps.ends[i];j+=getpagesize()) {
      //printf("address: 0x%08x\n",j);
      //printf("\t\tAddress: %p\n", j);
      if(lseek(memFd,j,SEEK_SET) == -1) {
        //printf("lseek error: %p, %s\n",j,strerror(errno));
        continue;
      }

      if(read(memFd,page,getpagesize()) == -1) {
        //printf("read error: %p, %s\n",j,strerror(errno));
        continue;
      }

      //Once get the page, now can compare the data within the page
      for(int k=0;k<=getpagesize() - size;k++) {
        //printf("Data: %p\n", page+k);
        if(memcmp(page+k, data, size) == 0) {
          scanner.addresses.push_back(j +k);
        }
      }
    }
  }

  printf("Found %lu results\n",scanner.addresses.size());
  free(page);
  close(memFd);

  pidDetach(pid);
}

void memScanFilter(Scanner &scanner,pid_t pid,unsigned char* data,int size) {
  pidAttach(pid);
  vector<unsigned long> addresses; //New addresses

  int memFd = getMem(pid);

  uint8_t* buf = (uint8_t*)malloc(size);

  for(unsigned int i=0;i<scanner.addresses.size();i++) {
    if(lseek(memFd,scanner.addresses[i],SEEK_SET) == -1) {
      //printf("lseek error: 0x%08x, %s\n",scanner.addresses[i],strerror(errno));
      continue;
    }
    if(read(memFd,buf,size) == -1) {
      //printf("read error: 0x%08x, %s\n",scanner.addresses[i],strerror(errno));
      continue;
    }

    if(memcmp(buf,data,size) == 0) {
      addresses.push_back(scanner.addresses[i]);
    }
  }


  free(buf);

  //Remove old one
  scanner.addresses = addresses;
  printf("Filtered %lu results\n",scanner.addresses.size());
  close(memFd);
  pidDetach(pid);
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

/**
 * Check whether the address is in the /proc/PID/maps
 */
bool addrInMaps(pid_t pid, unsigned long address) {
  ProcMaps maps = getMaps(pid);
  for(unsigned int i=0;i<maps.starts.size();i++) {
    if(address < maps.starts[i] || address > maps.ends[i]) {
      return false;
    }
  }

  return true;
}

bool addrIsValid(pid_t pid,unsigned long address) {

  return true;
}


string memValue(long pid, unsigned long address, string scanType) throw(MedException) {
  pidAttach(pid);

  int size = scanTypeToSize(stringToScanType(scanType));

  int memFd = getMem(pid);
  uint8_t* buf = (uint8_t*)malloc(size + 1); //+1 for the NULL
  memset(buf,0,size+1);

  medMutex.lock();

  if(lseek(memFd,address,SEEK_SET)==-1) {
    free(buf);
    close(memFd);
    pidDetach(pid);
    medMutex.unlock();
    throw MedException("Address seek fail");
  }
  if(read(memFd,buf,size) == -1) {
    free(buf);
    close(memFd);
    pidDetach(pid);
    medMutex.unlock();
    throw MedException("Address read fail");
  }

  char str[32];
  switch(stringToScanType(scanType)) {
  case Int8:
    sprintf(str,"%" PRIu8, *(uint8_t*)buf);
    break;
  case Int16:
    sprintf(str,"%" PRIu16, *(uint16_t*)buf);
    break;
  case Int32:
    sprintf(str,"%" PRIu32, *(uint32_t*)buf);
    break;
  case Float32:
    sprintf(str,"%f", *(float*)buf);
    break;
  case Float64:
    sprintf(str,"%lf", *(double*)buf);
    break;
  case Unknown:
    free(buf);
    close(memFd);
    pidDetach(pid);
    medMutex.unlock();
    throw MedException("Error Type");
  }

  free(buf);
  close(memFd);
  pidDetach(pid);

  string ret = string(str);
  medMutex.unlock();
  return ret;
}

/**
 * Get the list of PID
 * This is done by accessing the /proc and /proc/PID and /proc/PID/cmdline
 * The list suppose to be in the descending order
 */
vector<Process> pidList() {
  //Get directories
  vector<Process> pids;

  DIR *d;
  struct dirent *dir;
  d = opendir("/proc");
  if(d) {
    while((dir = readdir(d)) != NULL) {
      if(isdigit(dir->d_name[0])) {
        string cmd = pidName(dir->d_name);
        if(cmd.length()) {
          Process proc;
          proc.pid = dir->d_name;
          proc.cmdline = pidName(proc.pid);
          pids.push_back(proc);
        }

      }
    }
    closedir(d);
  }
  return pids;
}



/**
 * Get the cmdline from PID
 */
string pidName(string pid) {
  string ret;
  ifstream ifile;
  ifile.open(string("/proc/") + pid + "/cmdline");
  if(ifile.fail()) {
    return "";
  }
  ifile >> ret;
  ifile.close();
  \
  return ret;
}


/*******************
Object (though not oriented) solution
*****************/
MedScan::MedScan() {}
MedScan::MedScan(unsigned long address) {
  this->address = address;
}
string MedScan::getScanType() {
  return scanTypeToString(this->scanType);
}

void MedScan::setScanType(string s) {
  this->scanType = stringToScanType(s);
}

string MedScan::getValue(long pid) {
  string value;
  value = memValue(pid, this->address, this->getScanType());
  return value;
}

string MedScan::getValue(long pid, string scanType) {
  string value;
  value = memValue(pid, this->address, scanType);
  return value;
}

void MedScan::setValue(long pid, string v) {
  uint8_t* buffer;
  int size = stringToRaw(v, this->scanType, &buffer);
  memWrite(pid, this->address, buffer, size);
  free(buffer);
}


MedAddress::MedAddress() {
  this->lock = false;
  this->lockThread = NULL;
}
MedAddress::MedAddress(unsigned long address) : MedScan(address) {
  this->lock = false;
  this->lockThread = NULL;
}
MedAddress::~MedAddress() {
  if (this->lockThread) {
    this->unlockValue();
  }
}

void MedAddress::lockValue(string pid) {
  this->lock = true;
  this->lockedValue = this->getValue(stol(pid), this->getScanType());
  this->lockThread = new std::thread(::lockValue, pid, this);
}

void MedAddress::unlockValue() {
  this->lock = false;
  this->lockThread->join();
  delete this->lockThread;
  this->lockThread = NULL;
}


Med::Med() {}
Med::~Med() {
  clearStore();
}

void Med::scanEqual(string v, string t) {
  uint8_t* buffer = NULL;
  int size = stringToRaw(v, t, &buffer);
  Med::memScanEqual(this->scanAddresses, stoi(this->selectedProcess.pid), buffer, size, t);
  if(buffer)
    free(buffer);
}

void Med::scanFilter(string v, string t) {
  uint8_t* buffer = NULL;
  int size = stringToRaw(v, t, &buffer);
  Med::memScanFilter(this->scanAddresses, stoi(this->selectedProcess.pid), buffer, size, t);
  if(buffer)
    free(buffer);
}

/**
 * Scan memory, using procfs mem
 */
void Med::memScanEqual(vector<MedScan> &scanAddresses,pid_t pid,unsigned char* data,int size, string scanType = NULL) {
  medMutex.lock();
  pidAttach(pid);

  ProcMaps maps = getMaps(pid);

  uint8_t* page = (uint8_t*)malloc(getpagesize()); //For block of memory

  int memFd = getMem(pid);

  //Loop through maps
  scanAddresses.clear();
  for(unsigned int i=0;i<maps.starts.size();i++) {
    //printf("maps: %p\t%p\n", maps.starts[i],maps.ends[i]);
    //Loop each maps to get the "page"
    for(unsigned long int j=maps.starts[i];j<maps.ends[i];j+=getpagesize()) {
      //printf("address: 0x%08x\n",j);
      //printf("\t\tAddress: %p\n", j);
      if(lseek(memFd,j,SEEK_SET) == -1) {
        //printf("lseek error: %p, %s\n",j,strerror(errno));
        continue;
      }

      if(read(memFd,page,getpagesize()) == -1) {
        //printf("read error: %p, %s\n",j,strerror(errno));
        continue;
      }

      //Once get the page, now can compare the data within the page
      for(int k=0;k<=getpagesize() - size;k++) {
        //printf("Data: %p\n", page+k);
        if(memcmp(page+k, data, size) == 0) {
          MedScan medScan(j +k);
          medScan.setScanType(scanType);
          scanAddresses.push_back(medScan);
        }
      }
    }
  }

  printf("Found %lu results\n",scanAddresses.size());
  free(page);
  close(memFd);

  pidDetach(pid);
  medMutex.unlock();
}

void Med::memScanFilter(vector<MedScan> &scanAddresses,pid_t pid,unsigned char* data,int size, string scanType = NULL) {
  medMutex.lock();
  pidAttach(pid);
  vector<MedScan> addresses; //New addresses

  int memFd = getMem(pid);

  uint8_t* buf = (uint8_t*)malloc(size);

  for(unsigned int i=0;i<scanAddresses.size();i++) {
    if(lseek(memFd,scanAddresses[i].address,SEEK_SET) == -1) {
      //printf("lseek error: 0x%08x, %s\n",scanner.addresses[i],strerror(errno));
      continue;
    }
    if(read(memFd,buf,size) == -1) {
      //printf("read error: 0x%08x, %s\n",scanner.addresses[i],strerror(errno));
      continue;
    }

    if(memcmp(buf,data,size) == 0) {
      scanAddresses[i].setScanType(scanType);
      addresses.push_back(scanAddresses[i]);
    }
  }

  free(buf);

  //Remove old one
  scanAddresses = addresses;
  printf("Filtered %lu results\n",scanAddresses.size());
  close(memFd);
  pidDetach(pid);
  medMutex.unlock();
}

vector<Process> Med::listProcesses() {
  this->processes = pidList();
  return this->processes;
}

string Med::getScanAddressValueByIndex(int ind, string scanType) {
  return this->scanAddresses[ind].getValue(stol(this->selectedProcess.pid), scanType);
}

string Med::getScanValueByIndex(int ind) {
  return this->scanAddresses[ind].getValue(stol(this->selectedProcess.pid), this->scanAddresses[ind].getScanType());
}

string Med::getScanTypeByIndex(int ind) {
  return this->scanAddresses[ind].getScanType();
}

/**
 * @deprecated
 */
string Med:: getAddressValueByIndex(int ind) {
  return getStoreValueByIndex(ind);
}

string Med::getStoreValueByIndex(int ind) {
  return this->addresses[ind]->getValue(stol(this->selectedProcess.pid), this->addresses[ind]->getScanType());
}

string Med::getValueByAddress(unsigned long address, string scanType) {
  string value;
  //if(address != 0) //In case adding the address manually
  //  medMutex.lock();
  value = memValue(stol(this->selectedProcess.pid), address, scanType);
  //if(address != 0)
  //  medMutex.unlock();
  return value;
}

void Med::setValueByAddress(unsigned long address, string value, string scanType) {
  uint8_t* buffer = NULL;
  int size = stringToRaw(string(value), scanType, &buffer);
  memWrite(stoi(this->selectedProcess.pid), address, buffer, size);
  if(buffer)
    free(buffer);
}

bool Med::addToStoreByIndex(int index) {
  if (index < 0 || index > this->scanAddresses.size() - 1) {
    return false;
  }

  MedAddress* medAddress = new MedAddress();
  medAddress->description = "Your description";
  medAddress->address = this->scanAddresses[index].address;
  medAddress->scanType = this->scanAddresses[index].scanType;
  this->addresses.push_back(medAddress);

  return true;
}

void Med::saveFile(const char* filename) throw(MedException) {
  Json::Value root;
  //Using C++11
  for(auto address: this->addresses) {
    Json::Value pairs;
    pairs["description"] = address->description;
    pairs["address"] = intToHex(address->address);
    pairs["type"] = address->getScanType();
    pairs["value"] = string(address->getValue(stol(this->selectedProcess.pid), address->getScanType()));
    pairs["lock"] = address->lock;
    root.append(pairs);
  }
  ofstream ofs;
  ofs.open(filename);
  if(ofs.fail()) {
    throw MedException(string("Save JSON: Fail to open file ") + filename);
  }
  ofs << root << endl;
  ofs.close();
}

void Med::openFile(const char* filename) throw(MedException) {
  Json::Value root;

  ifstream ifs;
  ifs.open(filename);
  if(ifs.fail()) {
    throw MedException(string("Open JSON: Fail to open file ") + filename);
  }
  ifs >> root;
  ifs.close();

  clearStore();
  for(int i=0;i<root.size();i++) {
    MedAddress* address = new MedAddress();
    address->description = root[i]["description"].asString();
    address->address = hexToInt(root[i]["address"].asString());
    address->setScanType(root[i]["type"].asString());
    address->lock = false; // always open as false, so that do not update the value

    this->addresses.push_back(address);
  }
}

void lockValue(string pid, MedAddress* address) {
  while(1) {
    if(!address->lock) {
      return; //End
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    address->setValue(stol(pid), address->lockedValue);
  }
}

void Med::lockAddressValueByIndex(int ind) {
  //Create a thread
  MedAddress* address = this->addresses[ind];
  if (address->lock)
    return;
  address->lockValue(this->selectedProcess.pid);
}

void Med::unlockAddressValueByIndex(int ind) {
  MedAddress* address = this->addresses[ind];
  if (!address->lock)
    return;
  address->unlockValue();
}

void Med::setStoreLockByIndex(int ind, bool lockStatus) {
  lockStatus ? lockAddressValueByIndex(ind) : unlockAddressValueByIndex(ind);
}

bool Med::getStoreLockByIndex(int ind) {
  return addresses[ind]->lock;
}

void Med::addNewAddress() {
  MedAddress* medAddress = new MedAddress();
  medAddress->description = "Your description";
  medAddress->address = 0;
  medAddress->setScanType("int16");
  medAddress->lock = false;
  addresses.push_back(medAddress);
}

string Med::getScanAddressByIndex(int ind) {
  char address[32];
  sprintf(address, "%p", (void*)(scanAddresses[ind].address));
  return string(address);
}

string Med::getStoreAddressByIndex(int ind) {
  char address[32];
  sprintf(address, "%p", (void*)(addresses[ind]->address));
  return string(address);
}

void Med::deleteAddressByIndex(int ind) {
  delete addresses[ind];
  addresses.erase(addresses.begin() + ind);
}

void Med::shiftStoreAddresses(long diff) {
  for (int i=0; i < addresses.size(); i++) {
    long address = addresses[i]->address;
    address += diff;
    addresses[i]->address = address;
  }
}

string Med::getStoreDescriptionByIndex(int ind) {
  return addresses[ind]->description;
}

string Med::setStoreAddressByIndex(int ind, string address) {
  addresses[ind]->address = hexToInt(address);
  string value = getValueByAddress(addresses[ind]->address, addresses[ind]->getScanType());
  return value;
}

void Med::clearStore() {
  for(unsigned int i=0;i<addresses.size();i++)
    delete addresses[i];
  addresses.clear();
}

void Med::setStoreDescriptionByIndex(int ind, string description) {
  addresses[ind]->description = description;
}

void Med::sortStoreByAddress() {
  sort(addresses.begin(), addresses.end(), [](MedAddress* a, MedAddress* b) {
      return a->address < b->address;
    });
}

void Med::sortStoreByDescription() {
  sort(addresses.begin(), addresses.end(), [](MedAddress* a, MedAddress* b) {
      return a->description.compare(b->description) < 0; 
    });
}
