#ifndef MEM_ED_HPP
#define MEM_ED_HPP

#include <mutex>
#include <thread>

#include <json/json.h>

#include "mem/MemScanner.hpp"
#include "mem/MemList.hpp"
#include "mem/NamedScans.hpp"
#include "med/Process.hpp"

const int LOCK_REFRESH_RATE = 800;

class MemEd {
public:
  MemEd();
  explicit MemEd(pid_t pid);
  ~MemEd();
  void setPid(pid_t pid);
  pid_t getPid();
  vector<MemPtr> scan(const string& value, const string& scanType, bool fastScan = false, const string& lastDigit = "");
  vector<MemPtr> filter(const string& value, const string& scanType, bool fastScan = false);
  MemList getScans();
  void clearScans();
  MemList* getStore();
  void addToStoreByIndex(int index);
  void addNewAddress();
  MemPtr readMemory(Address addr, size_t size);
  void setValueByAddress(Address addr, const string& value, const string& scanType);

  // Process
  vector<Process> listProcesses();
  Process selectProcessByIndex(int index);
  vector<Process> processes;
  Process selectedProcess;

  void lockValues();
  bool hasLockValue();

  static void callLockValues(MemEd* med);

  void saveFile(const char* filename);
  void openFile(const char* filename);
  void loadLegacyJson(Json::Value& root);
  void loadJson(Json::Value& root);

  string& getNotes();
  void setNotes(const string& notes);

  void setScopeStart(Address addr);
  void setScopeEnd(Address addr);

  std::mutex& getScanListMutex();

  void resumeProcess();
  void pauseProcess();
  bool getIsProcessPaused();
  void setCanResumeProcess(bool value);
  bool getCanResumeProcess();

private:
  void initialize();
  pid_t pid;
  MemScanner* scanner;
  NamedScans namedScans;
  MemList* store;
  std::mutex storeMutex;
  std::thread* lockValueThread;
  bool canResumeProcess;
  bool isProcessPaused;

  string notes;
};

#endif
