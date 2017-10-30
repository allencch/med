#include <iostream>
#include <sstream>
#include <cstring> //strerror()
#include <fstream>

#include <fcntl.h> //open, read, lseek
#include <sys/ptrace.h> //ptrace()
#include <sys/wait.h> //waitpid()
#include <dirent.h> //read directory

#include "med/MedCommon.hpp"
#include "med/MedException.hpp"
#include "med/ScanParser.hpp"
#include "med/Process.hpp"
#include "med/med.hpp"

using namespace std;

/**
 * @brief Convert hexadecimal string to integer value
 */
long hexToInt(string str) {
  stringstream ss(str);
  long ret = -1;
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
  if (scanType == SCAN_TYPE_INT_8) {
    return Int8;
  }
  else if (scanType == SCAN_TYPE_INT_16) {
    return Int16;
  }
  else if (scanType == SCAN_TYPE_INT_32) {
    return Int32;
  }
  else if (scanType == SCAN_TYPE_FLOAT_32) {
    return Float32;
  }
  else if (scanType == SCAN_TYPE_FLOAT_64) {
    return Float64;
  }
  else if (scanType == SCAN_TYPE_STRING) {
    return String;
  }
  return Unknown;
}

string scanTypeToString(ScanType scanType) {
  string ret;
  switch (scanType) {
  case Int8:
    ret = SCAN_TYPE_INT_8;
    break;
  case Int16:
    ret = SCAN_TYPE_INT_16;
    break;
  case Int32:
    ret = SCAN_TYPE_INT_32;
    break;
  case Float32:
    ret = SCAN_TYPE_FLOAT_32;
    break;
  case Float64:
    ret = SCAN_TYPE_FLOAT_64;
    break;
  case String:
    ret = SCAN_TYPE_STRING;
    break;
  default:
    ret = SCAN_TYPE_UNKNOWN;
  }
  return ret;
}

int scanTypeToSize(ScanType type) {
  int ret = 0;
  switch (type) {
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
  case String:
    ret = MAX_STRING_SIZE;
    break;
  case Unknown:
    ret = 0;
  }
  return ret;
}

int scanTypeToSize(string type) {
  return scanTypeToSize(stringToScanType(type));
}

int createBufferByScanType(ScanType type, void** buffer, int size) {
  int retsize = scanTypeToSize(type) * size;
  void* buf = NULL;
  switch (type) {
  case Int8:
    retsize = sizeof(uint8_t) * size;
    buf = new Byte[sizeof(uint8_t) * size];
    break;
  case Int16:
    retsize = sizeof(uint16_t) * size;
    buf = new Byte[sizeof(uint16_t) * size];
    break;
  case Int32:
    retsize = sizeof(uint32_t) * size;
    buf = new Byte[sizeof(uint32_t) * size];
    break;
  case Float32:
    retsize = sizeof(float) * size;
    buf = new Byte[sizeof(float) * size];
    break;
  case Float64:
    retsize = sizeof(double) * size;
    buf = new Byte[sizeof(double) * size];
    break;
  case String:
    retsize = 1;
    break;
  case Unknown:
    retsize = 0;
    break;
  }
  *buffer = buf;

  return retsize;
}


int stringToRaw(string str, ScanType type, Byte** buffer) {
  int retsize;
  if (type == ScanType::String) {
    retsize = stringToRawStringType(str, type, buffer);;
  }
  else {
    retsize = stringToRawNumericType(str, type, buffer);
  }

  return retsize;
}

int stringToRawStringType(string str, ScanType type, Byte** buffer) {
  Bytes bytes = ScanParser::getBytes(str, scanTypeToString(type));
  int retsize = bytes.size;
  Byte* buf = bytes.data;

  char* pointer = (char*)buf;
  for (int i = 0; i < (int)str.size(); i++, pointer++) {
    sprintf(pointer, "%c", str[i]);
  }
  *buffer = buf;

  return retsize;
}

int stringToRawNumericType(string str, ScanType type, Byte** buffer) {
  vector<string> tokens = ScanParser::getValues(str);

  int retsize;
  Byte* buf;
  int size = tokens.size();

  retsize = createBufferByScanType(type, (void**)buffer, size);
  buf = *buffer;
  for (unsigned int i = 0; i < tokens.size(); i++) {
    stringToMemory(tokens[i], type, buf);
    buf += scanTypeToSize(type);
  }
  return retsize;
}


int stringToRaw(string str, string type, Byte** buffer) {
  ScanType scantype = stringToScanType(type);
  return stringToRaw(str, scantype, buffer);
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
  MemAddr start, end;
  char rd, wr;
  int inode;
  char sp; //shared or private
  char fname[128];

  //Get line
  char line[256];

  while(fgets(line,255,file)) {
    //parse line
    //the empty pathname has to be scan also
    if (sscanf(line, "%lx-%lx %c%c%c%c %s %s %u %s",
              &start, &end,
              &rd, &wr, useless, &sp,
              useless, useless, &inode, fname) < 9) {
      continue;
    }

    if (rd == 'r' && wr == 'w' && ((end - start) > 0)) {
      procMaps.starts.push_back(start);
      procMaps.ends.push_back(end);
    }
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
    printf("Open failed: %s\n", strerror(errno));
  }
  return ret;
}


/**
 * Attach PID
 */
pid_t pidAttach(pid_t pid) {
  if(ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1L) {
    fprintf(stderr, "Failed attach: %s\n", strerror(errno));
    throw MedException("Failed attach");
  }

  int status;
  if(waitpid(pid, &status, 0) == -1 || !WIFSTOPPED(status)) {
    fprintf(stderr, "Error waiting: %s\n", strerror(errno));
    throw MedException("Error waiting");
  }

  return pid;
}

pid_t pidDetach(pid_t pid){
  if(ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1L) {
    fprintf(stderr, "Failed detach: %s\n", strerror(errno));
    throw MedException("Failed detach");
  }
  return -1;
}


/**
 * Convert the size to padded word size.
 */
int padWordSize(int x) {
  if(x % sizeof(MemAddr))
    return x + sizeof(MemAddr) - x % sizeof(MemAddr);
  return x;
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

  return ret;
}

void lockValue(string pid, MedAddress* address) {
  while(1) {
    if(!address->lock) {
      return; //End
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(REFRESH_RATE));
    try {
      address->setValue(stol(pid), address->getLockedValue());
    } catch(MedException& ex) {
      cerr << "Lock value failed: " << ex.getMessage() << endl;
    }
  }
}


/**
 * This will just perform the unlock by force
 */
void tryUnlock(std::mutex &mutex) {
  mutex.try_lock();
  mutex.unlock();
}

void stringToMemory(const string& str, const ScanType& type, Byte* buffer) {
  stringstream ss(str);

  int temp;
  switch (type) {
  case Int8:
    ss >> dec >> temp; //Because it does not handle the uint8_t correctly as integer, but a character.
    *(uint8_t*)buffer = (uint8_t) temp;
    break;
  case Int16:
    ss >> dec >> *(uint16_t*)buffer;
    break;
  case Int32:
    ss >> dec >> *(uint32_t*)buffer;
    break;
  case Float32:
    ss >> dec >> *(float*)buffer;
    break;
  case Float64:
    ss >> dec >> *(double*)buffer;
    break;
  case String:
  case Unknown:
    break;
  }
  if(ss.fail()) {
    printf("Error input: %s\n", str.c_str());
  }
}

void stringToMemory(const string& str, const string& type, Byte* buffer) {
  ScanType scanType = stringToScanType(type);
  stringToMemory(str, scanType, buffer);
}
