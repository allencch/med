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
 * TODO: 2016-05-22
 * Re-design into object, so that can work smoothly with the GUI.
 * Previously, I focused on the scanner only. Then modify the value or whatever
 * happened at the GUI. Now I plan to move every thing to the object, then the frontend
 * will be designed based on the object.
 * But I don't intend to put mutex here. (Will think about it)
 *
 * TODO: (old)
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
 * Important:
 * pidAttach() must be attach only when scan, else the process is being paused.
 * Regarding "word" in ptrace(), the size of "word" is depending on the architecture. But ptrace() itself is using long. So, "long" datatype is the solution for the word size.
 */


#ifndef MED_H
#define MED_H

#include <string>
#include <mutex>
#include <thread>
#include <exception>

using namespace std;

static std::mutex medMutex; //One and only one, globally accessible

struct ProcMaps {
  vector<unsigned long> starts;
  vector<unsigned long> ends;
};

struct Process {
  string pid;
  string cmdline; //aka "process" in GUI
};

struct Scanner {
  vector<unsigned long> addresses;
};

enum ScanType {
  Int8,
  Int16,
  Int32,
  Float32,
  Float64,
  Unknown
};

class MedException: public exception {
public:
  MedException(string message) {
    this->message = message;
  }
  virtual const char* what() const throw() {
    return message.c_str();
  }
private:
  string message;
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
long hexToInt(string str) throw(MedException);

/**
 * @brief Convert long integer to hex string
 */
string intToHex(long int);


/**
 * @brief Print the hexadecimal data
 */
void printHex(FILE* file,void* addr,int size);

/**
 * @param pid is pid_t, which is actually integer.
 */
ProcMaps getMaps(pid_t pid);

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

pid_t pidAttach(pid_t pid) throw(MedException);
pid_t pidDetach(pid_t pid) throw(MedException);

int memDump(pid_t pid, unsigned long address,int size);
void memWrite(pid_t pid, unsigned long address,uint8_t* data,int size);
void memWriteList(Scanner scanner,pid_t pid,uint8_t* data,int size);

int memCopy(pid_t pid,unsigned long address,unsigned char* out,int size,bool showError);

/**
 * Scan memory, using procfs mem
 * @param pid (pid_t) is an integer of PID
 */
void memScanEqual(Scanner &scanner,pid_t pid,unsigned char* data,int size);
void memScanFilter(Scanner &scanner,pid_t pid,unsigned char* data,int size);

/**
 * Reverse the memory (Big to Little Endian or vice versa)
 */
void memReverse(uint8_t* buf,int size);

/**
 * Get the list of PID
 * This is done by accessing the /proc and /proc/PID and /proc/PID/cmdline
 * The list suppose to be in the descending order
 */
vector<Process> pidList();

/**
 * Get the cmdline from PID
 */
string pidName(string pid);

/**
 * Get the value from the address as string based on the scanType (string)
 */
string memValue(long pid, unsigned long address, string scanType) throw(MedException);


/**
 * This is the Scanner replacement
 */
class MedScan {
public:
  MedScan();
  MedScan(unsigned long address);
  unsigned long address;

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
  MedAddress(unsigned long address);
  ~MedAddress();

  //TODO: Unlock and join in destructor

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


  void scanEqual(string scanValue, string scanType) throw(MedException);
  void scanFilter(string scanValue, string scanType) throw(MedException);

  vector<Process> listProcesses();

  string getScanAddressValueByIndex(int ind, string scanType);
  string getScanValueByIndex(int ind);
  string getScanTypeByIndex(int ind);

  /**
   * @deprecated. Replace by getStoreValueByIndex()
   * The function name is not value significant. It refers to right hand panel stored address.
   */
  string getAddressValueByIndex(int ind);
  string getStoreValueByIndex(int ind);

  string getValueByAddress(unsigned long address, string scanType);
  void setValueByAddress(unsigned long address, string value, string scanType);


  void setStoreLockByIndex(int ind, bool lockStatus);
  bool getStoreLockByIndex(int ind);
  void lockAddressValueByIndex(int ind);
  void unlockAddressValueByIndex(int ind);

  void saveFile(const char* filename) throw(MedException);
  void openFile(const char* filename) throw(MedException);

  bool addToStoreByIndex(int ind);

  void addNewAddress();
  void deleteAddressByIndex(int ind);
  void shiftStoreAddresses(long diff);

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
  /**
   * @param scanType is just a record.
   */
  static void memScanEqual(vector<MedScan> &scanAddresses,pid_t pid,unsigned char* data,int size, string scanType);
  static void memScanFilter(vector<MedScan> &scanAddresses,pid_t pid,unsigned char* data,int size, string scanType);
};

void lockValue(string pid, MedAddress* address);

#endif
