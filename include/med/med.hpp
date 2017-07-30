/**
 * @author	Allen Choong
 * @version	0.0.1
 * @date	2012-03-15
 *
 * Simple memory editor. The goal is to hack the Android game from adb shell
 * It is inspired by scanmem.
 * Actually intend to write in C, but I need list (vector), so I change
 *
 * TODO: (old)
 * Scan integer, float, little endian, big endian, arrays
 * Edit integer, float, little endian, big endian, arrays
 * Dump with hex position
 * Pause and play the process
 *
 *
 * Search by block
 *
 * Important:
 * pidAttach() must be attach only when scan, else the process is being paused.
 * Regarding "word" in ptrace(), the size of "word" is depending on the architecture. But ptrace() itself is using long. So, "long" datatype is the solution for the word size.
 */


#ifndef MED_H
#define MED_H

#include <string>
#include <mutex>
#include <thread>
#include <vector>
#include <exception>

#include "med/MedException.hpp"
#include "med/ScanParser.hpp"
#include "med/ThreadManager.hpp"

using namespace std;

static std::mutex medMutex; //One and only one, globally accessible

typedef unsigned long MemAddr;
typedef unsigned char Byte;

struct ProcMaps {
  vector<MemAddr> starts;
  vector<MemAddr> ends;
};

struct Process {
  string pid;
  string cmdline; //aka "process" in GUI
};


enum ScanType {
  Int8,
  Int16,
  Int32,
  Float32,
  Float64,
  Unknown
};



/**
 * Convert string to ScanType, they are "int8", "int16", etc.
 */
ScanType stringToScanType(string scanType);

/**
 * Convert ScanType to sizeof
 */
int scanTypeToSize(ScanType type);

/**
 * @brief Convert hexadecimal string to integer value
 */
long hexToInt(string str);

/**
 * @brief Convert long integer to hex string
 */
string intToHex(long int);

/**
 * @brief Print the hexadecimal data
 */
void printHex(FILE* file, void* addr, int size);

/**
 * @param pid is pid_t, which is actually integer.
 */
ProcMaps getMaps(pid_t pid);

// Create buffer by scan type. Remember to free()
int createBufferByScanType(ScanType type, void** buffer, int size = 1);

/**
 * Convert numerical string to raw data (hexadecimal).
 * @param s is a string which can separated by space
 * @param buffer is to store the output address, must be free() if not used.
 * @return number of values to be scanned
 */
int stringToRaw(string str, ScanType type, uint8_t** buffer);

/**
 * @param type is string in "int8", "int16", etc
 */
int stringToRaw(string str, string type, uint8_t** buffer);

/**
 * Convert the size to padded word size.
 */
int padWordSize(int x);

/**
 * Open the /proc/[pid]/mem
 * @return file descriptor
 */
int getMem(pid_t pid);

pid_t pidAttach(pid_t pid);
pid_t pidDetach(pid_t pid);


/**
 * Get the cmdline from PID
 */
string pidName(string pid);


/**
 * Lock value thread
 */
class MedAddress; //Prototype
void lockValue(string pid, MedAddress* address);



/**
 * This is the Scanner replacement
 */
class MedScan {
public:
  MedScan();
  MedScan(MemAddr address);
  MemAddr address;

  string getScanType();
  void setScanType(string s);
  ScanType scanType; //direct access

  //No value, because value is calculated based on scan type
  string getValue(long pid);
  string getValue(long pid, string scanType);
  void setValue(long pid, string val);
};

/**
 * This is the address will be saved, re-use, lock, etc.
 */
class MedAddress : public MedScan {
  friend class Med;
public:
  MedAddress();
  MedAddress(MemAddr address);
  ~MedAddress();

  string description;
  bool lock; /**< Not using mutex */
  string lockedValue;

  void lockValue(string pid);
  void unlockValue();

private:
  std::thread* lockThread;
};

/**
 * This is the core scanner. Only one scanner
 */
class Med {
public:
  Med();
  virtual ~Med();

  vector<Process> processes; /**< This is the list of processes */
  vector<MedScan> scanAddresses; /**< The memory scanner, replace the Scanner*/
  vector<MedAddress*> addresses;

  void clearStore();

  Process selectedProcess; /**< Not using pointer yet */

  void scan(string scanValue, string scanType);
  void filter(string scanValue, string scanType);

  vector<Process> listProcesses();

  string getScanAddressValueByIndex(int ind, string scanType);
  string getScanValueByIndex(int ind);
  string getScanTypeByIndex(int ind);

  string getStoreValueByIndex(int ind);

  string getValueByAddress(MemAddr address, string scanType);
  void setValueByAddress(MemAddr address, string value, string scanType);


  void setStoreLockByIndex(int ind, bool lockStatus);
  bool getStoreLockByIndex(int ind);
  void lockAddressValueByIndex(int ind);
  void unlockAddressValueByIndex(int ind);

  void saveFile(const char* filename);
  void openFile(const char* filename);

  bool addToStoreByIndex(int ind);

  void addNewAddress();
  void deleteAddressByIndex(int ind);
  void shiftStoreAddresses(long diff);
  void shiftStoreAddressByIndex(int ind, long diff);

  /**
   * @return string in the hex format
   */
  string getScanAddressByIndex(int ind);
  string getStoreAddressByIndex(int ind);

  string setStoreAddressByIndex(int ind, string address);

  string getStoreDescriptionByIndex(int ind);

  void setStoreDescriptionByIndex(int ind, string description);

  void sortStoreByDescription();
  void sortStoreByAddress();

private:
  static void memScan(vector<MedScan> &scanAddresses,
                      pid_t pid,
                      Byte* data,
                      int size,
                      string scanType,
                      ScanParser::OpType op);

  static void memScanPage(uint8_t* page,
                          MemAddr start,
                          int srcSize,
                          vector<MedScan> &scanAddresses,
                          Byte* data,
                          int size,
                          string scanType,
                          ScanParser::OpType op);
  static void memScanMap(ProcMaps& maps,
                         int mapIndex,
                         int fd,
                         uint8_t* page,
                         int srcSize,
                         vector<MedScan> &scanAddresses,
                         Byte* data,
                         int size,
                         string scanType,
                         ScanParser::OpType op);

  static void memFilter(vector<MedScan> &scanAddresses, pid_t pid, Byte* data, int size, string scanType, ScanParser::OpType op);

  ThreadManager* threadManager;
};


#endif
