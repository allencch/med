#ifndef MED_H
#define MED_H

#include <string>
#include <mutex>
#include <thread>
#include <vector>
#include <exception>

#include <json/json.h>

#include "med/MedTypes.hpp"
#include "med/MedScan.hpp"
#include "med/MedAddress.hpp"
#include "med/MedException.hpp"
#include "med/MedCommon.hpp"
#include "med/ScanParser.hpp"
#include "med/ThreadManager.hpp"
#include "med/Snapshot.hpp"
#include "med/Process.hpp"


using namespace std;

extern std::mutex medMutex; //One and only one, globally accessible

const int REFRESH_RATE = 800;
const int SCAN_ADDRESS_VISIBLE_SIZE = 800;


/**
 * This is the core scanner. Only one scanner
 */
class Med {
public:
  Med();
  virtual ~Med();

  vector<Process> processes;
  vector<MedScan> scanAddresses;
  vector<MedAddress*> addresses;
  Snapshot* snapshot;

  void clearStore();
  void clearScan();

  Process selectedProcess;

  void scan(string scanValue, string scanType);
  void filter(string scanValue, string scanType);

  vector<Process> listProcesses();
  string notes;

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
  void loadLegacyJson(Json::Value& root);
  void loadJson(Json::Value& root);

  bool addToStoreByIndex(int ind);

  MedAddress* addNewAddress();
  MedAddress* duplicateAddress(int ind);
  MedAddress* addNextAddress(int ind);
  MedAddress* addPrevAddress(int ind);
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

  void sortScanByAddress();

  Byte* readMemory(MemAddr address, size_t size); // need to free

private:
  static void memScan(Med* med,
                      vector<MedScan> &scanAddresses,
                      pid_t pid,
                      Byte* data,
                      int size,
                      string scanType,
                      ScanParser::OpType op);

  static void memScanPage(Med* med,
                          uint8_t* page,
                          MemAddr start,
                          int srcSize,
                          vector<MedScan> &scanAddresses,
                          Byte* data,
                          int size,
                          string scanType,
                          ScanParser::OpType op);
  static void memScanMap(Med* med,
                         ProcMaps& maps,
                         int mapIndex,
                         int fd,
                         int srcSize,
                         vector<MedScan> &scanAddresses,
                         Byte* data,
                         int size,
                         string scanType,
                         ScanParser::OpType op);

  static void memFilter(vector<MedScan> &scanAddresses, pid_t pid, Byte* data, int size, string scanType, ScanParser::OpType op);

  ThreadManager* threadManager;

  std::mutex scanAddressMutex;
};


#endif
