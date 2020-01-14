#include <iostream>
#include <sstream>
#include <cstring> //strerror()
#include <fstream>
#include <regex>

#include <fcntl.h> //open, read, lseek
#include <sys/ptrace.h> //ptrace()
#include <sys/wait.h> //waitpid()
#include <dirent.h> //read directory
#include <signal.h> // kill

#include "med/MedCommon.hpp"
#include "med/MedException.hpp"
#include "med/Process.hpp"
#include "med/ScanParser.hpp"
#include "mem/StringUtil.hpp"

using namespace std;

int hexStrToInt(const string& str) {
  auto trimmed = StringUtil::toLower(StringUtil::trim(str));
  if (trimmed == "0") return 0;
  else if (trimmed == "1") return 1;
  else if (trimmed == "2") return 2;
  else if (trimmed == "3") return 3;
  else if (trimmed == "4") return 4;
  else if (trimmed == "5") return 5;
  else if (trimmed == "6") return 6;
  else if (trimmed == "7") return 7;
  else if (trimmed == "8") return 8;
  else if (trimmed == "9") return 9;
  else if (trimmed == "a") return 10;
  else if (trimmed == "b") return 11;
  else if (trimmed == "c") return 12;
  else if (trimmed == "d") return 13;
  else if (trimmed == "e") return 14;
  else if (trimmed == "f") return 15;
  return -1;
}

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


ScanType stringToScanType(const string& scanType) {
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

string scanTypeToString(const ScanType& scanType) {
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

int scanTypeToSize(const ScanType& type) {
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

int scanTypeToSize(const string& type) {
  return scanTypeToSize(stringToScanType(type));
}

/**
 * @brief Print the hexadecimal data
 */
void printHex(FILE* file,void* addr,int size) {
  for(int i=0;i<size;i++) {
    fprintf(file,"%02x ",((uint8_t*)addr)[i]);
  }
}

Maps getMaps(pid_t pid) {
  Maps maps;

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
  Address start, end;
  char rd, wr;
  unsigned int inode;
  char sp; //shared or private
  char fname[128];

  //Get line
  char line[256];

  while (fgets(line, 255, file)) {
    //parse line
    //the empty pathname has to be scan also
    if (sscanf(line, "%lx-%lx %c%c%c%c %8s %5s %u %127s",
              &start, &end,
              &rd, &wr, useless, &sp,
              useless, useless, &inode, fname) < 9) {
      continue;
    }

    if (rd == 'r' && wr == 'w' && ((end - start) > 0)) {
      AddressPair pair(start, end);
      maps.push(pair);
    }
  }

  fclose(file);

  return maps;
}

/**
 * Open the /proc/[pid]/mem
 * @return file descriptor
 */
int getMem(pid_t pid) {
  char filename[32];
  sprintf(filename, "/proc/%d/mem", pid);
  int ret = open(filename, O_RDONLY);
  if (ret == -1) {
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

pid_t pidDetach(pid_t pid) {
  if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1L) {
    fprintf(stderr, "Failed detach: %s\n", strerror(errno));
    throw MedException("Failed detach");
  }
  return -1;
}

char getPidStatus(const char* stat) {
  string s = stat;
  regex reg("\\d+ \\(.*?\\) (\\w) \\d+");
  smatch m;
  if (regex_search(s, m, reg)) {
    return m.str(1).at(0);
  }
  return 'X';
}


bool isPidSuspended(pid_t pid) {
  char filename[128];
  sprintf(filename, "/proc/%d/stat", pid);
  FILE* file;
  file = fopen(filename, "r");
  if (!file) {
    printf("Failed open stat: %s\n", filename);
    exit(1);
  }
  char line[256];
  fgets(line, 255, file);
  fclose(file);

  char state = getPidStatus(line);
  if (state == 'T' || state == 't') {
    return true;
  }
  return false;
}

int pidResume(pid_t pid) {
  return kill(pid, SIGCONT);
}

int pidStop(pid_t pid) {
  return kill(pid, SIGSTOP);
}


/**
 * Convert the size to padded word size.
 */
int padWordSize(int x) {
  if(x % sizeof(Address))
    return x + sizeof(Address) - x % sizeof(Address);
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
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if (isdigit(dir->d_name[0])) {
        string cmd = pidName(dir->d_name);
        if (cmd.length()) {
          Process proc;
          proc.pid = dir->d_name;
          proc.cmdline = cmd;
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
string pidName(const string& pid) {
  string ret;
  ifstream ifile;
  ifile.open(string("/proc/") + pid + "/cmdline");
  if(ifile.fail()) {
    return "";
  }
  getline(ifile, ret);
  ifile.close();

  return ret;
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
    printf("Warning: stringToMemory with String type\n");
    break;
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

