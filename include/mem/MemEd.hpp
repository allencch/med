#ifndef MEM_ED_HPP
#define MEM_ED_HPP

#include <mutex>
#include <thread>

#include <json/json.h>

#include "mem/MemScanner.hpp"
#include "mem/MemManager.hpp"
#include "mem/MemList.hpp"
#include "med/Process.hpp"

const int LOCK_REFRESH_RATE = 800;

class MemEd {
public:
  MemEd();
  explicit MemEd(pid_t pid);
  ~MemEd();
  void setPid(pid_t pid);
  pid_t getPid();
  vector<MemPtr> scan(const string& value, const string& scanType);
  vector<MemPtr> filter(const string& value, const string& scanType);
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

  static void callLockValues(MemEd* med);

  void saveFile(const char* filename);
  void openFile(const char* filename);
  void loadLegacyJson(Json::Value& root);
  void loadJson(Json::Value& root);

  string& getNotes();
  void setNotes(const string& notes);

  void setScopeStart(Address addr);
  void setScopeEnd(Address addr);

private:
  void initialize();
  pid_t pid;
  MemScanner* scanner;
  MemManager* manager;
  MemList* store;
  std::mutex storeMutex;
  std::thread* lockValueThread;

  string notes;
};

#endif
