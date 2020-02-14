#include <iostream>
#include <fstream>

#include "mem/MemEd.hpp"
#include "med/MedCommon.hpp"
#include "med/MedException.hpp"
#include "mem/Sem.hpp"

using namespace std;

MemEd::MemEd() {
  initialize();
}

MemEd::MemEd(pid_t pid) {
  initialize();
  scanner->setPid(pid);
}

MemEd::~MemEd() {
  delete scanner;
  delete manager;

  delete store;

  lockValueThread->join();
  delete lockValueThread;
}

void MemEd::initialize() {
  pid = 0;
  scanner = new MemScanner();
  manager = new MemManager();

  vector<MemPtr> emptyMems;
  store = new MemList(emptyMems);
  canResumeProcess = true;
  isProcessPaused = false;

  lockValueThread = new std::thread(MemEd::callLockValues, this);
}

void MemEd::setPid(pid_t pid) {
  this->pid = pid;
  scanner->setPid(pid);
}

pid_t MemEd::getPid() {
  return pid;
}

vector<MemPtr> MemEd::scan(const string& value, const string& scanType, bool fastScan, const string& lastDigit) {
  if (!ScanParser::isValid(value)) {
    throw MedException("Invalid scan string");
  }

  ScanParser::OpType op = ScanParser::getOpType(value);

  vector<MemPtr> mems;
  if (op == ScanParser::OpType::SnapshotSave) {
    scanner->saveSnapshot(store->getList());
  }
  else {
    auto buffer = ScanParser::valueToBytes(value, scanType);
    size_t size = std::get<1>(buffer);

    int lastDigitValue = hexStrToInt(lastDigit);
    mems = scanner->scan(std::get<0>(buffer).get(), size, scanType, op, fastScan, lastDigitValue);
  }
  manager->setMems(mems);
  return mems;
}

vector<MemPtr> MemEd::filter(const string& value, const string& scanType) {
  if (!ScanParser::isValid(value)) {
    throw MedException("Invalid scan string");
  }

  vector<MemPtr> mems;
  ScanParser::OpType op = ScanParser::getOpType(value);
  if (ScanParser::isSnapshotOperator(op) && !ScanParser::hasValues(value)) {
    mems = scanner->filterUnknown(manager->getMems(), scanType, op);
  }
  else {
    auto buffer = ScanParser::valueToBytes(value, scanType);
    size_t size = std::get<1>(buffer);

    mems = scanner->filter(manager->getMems(), std::get<0>(buffer).get(), size, scanType, op);
  }

  manager->setMems(mems);
  return mems;
}

MemList MemEd::getScans() {
  MemList list(manager->getMems());
  return list;
}

vector<Process> MemEd::listProcesses() {
  processes = pidList();
  return processes;
}

Process MemEd::selectProcessByIndex(int index) {
  selectedProcess = processes[index];
  setPid(stoi(selectedProcess.pid));
  return selectedProcess;
}

void MemEd::clearScans() {
  manager->clear();
}

MemList* MemEd::getStore() {
  return store;
}

void MemEd::addToStoreByIndex(int index) {
  auto scans = getScans();
  PemPtr pem = static_pointer_cast<Pem>(scans.getMemPtr(index));
  SemPtr sem = Sem::convertToSemPtr(pem);
  getStore()->addMemPtr(sem);
}

void MemEd::callLockValues(MemEd* med) {
  while (1) {
    if (med->hasLockValue()) {
      med->lockValues();
      if (!med->getIsProcessPaused() && med->getCanResumeProcess()) {
        med->resumeProcess();
      }
    }
    std::this_thread::sleep_for(chrono::milliseconds(LOCK_REFRESH_RATE));
  }
}

void MemEd::lockValues() {
  storeMutex.lock();
  auto list = getStore()->getList();
  for (size_t i = 0; i < list.size(); i++) {
    auto sem = static_pointer_cast<Sem>(list[i]);
    if (sem->isLocked()) {
      sem->lockValue();
    }
  }
  storeMutex.unlock();
}

bool MemEd::hasLockValue() {
  auto list = getStore()->getList();
  for (size_t i = 0; i < list.size(); i++) {
    auto sem = static_pointer_cast<Sem>(list[i]);
    if (sem->isLocked()) {
      return true;
    }
  }
  return false;
}


void MemEd::saveFile(const char* filename) {
  Json::Value root;
  Json::Value addresses;
  root["addresses"] = addresses;

  auto list = getStore()->getList();
  for (auto address: list) {
    auto sem = static_pointer_cast<Sem>(address);
    Json::Value pairs;
    pairs["description"] = sem->getDescription();
    pairs["address"] = sem->getAddressAsString();
    pairs["type"] = sem->getScanType();
    try {
      pairs["value"] = sem->getValue();
    } catch(MedException &ex) {
      pairs["value"] = "";
    }
    pairs["lock"] = sem->isLocked();
    root["addresses"].append(pairs);
  }
  root["notes"] = getNotes();

  ofstream ofs;
  ofs.open(filename);
  if (ofs.fail()) {
    throw MedException(string("Save JSON: Fail to open file ") + filename);
  }
  ofs << root << endl;
  ofs.close();
}

void MemEd::loadLegacyJson(Json::Value& root) {
  MemIO* memio = scanner->getMemIO();
  for (int i = 0; i < (int)root.size(); i++) {
    string scanType = root[i]["type"].asString();
    int size = scanTypeToSize(scanType);

    SemPtr sem = SemPtr(new Sem(size, memio));
    sem->setAddress(hexToInt(root[i]["address"].asString()));
    sem->setScanType(scanType);
    sem->setDescription(root[i]["description"].asString());
    sem->lock(false); // always open as false, so that do not update the value

    getStore()->getList().push_back(sem);
  }
}

void MemEd::loadJson(Json::Value& root) {
  MemIO* memio = scanner->getMemIO();
  auto& addresses = root["addresses"];
  for (int i = 0; i < (int)addresses.size(); i++) {
    string scanType = addresses[i]["type"].asString();
    int size = scanTypeToSize(scanType);

    SemPtr sem = SemPtr(new Sem(size, memio));
    sem->setAddress(hexToInt(addresses[i]["address"].asString()));
    sem->setScanType(scanType);
    sem->setDescription(addresses[i]["description"].asString());
    sem->lock(false); // always open as false, so that do not update the value

    getStore()->getList().push_back(sem);
  }
  notes = root["notes"].asString();
}

void MemEd::openFile(const char* filename) {
  Json::Value root;

  ifstream ifs;
  ifs.open(filename);
  if (ifs.fail()) {
    throw MedException(string("Open JSON: Fail to open file ") + filename);
  }
  ifs >> root;
  ifs.close();

  storeMutex.lock();
  getStore()->clear();
  if (root.isArray()) {
    loadLegacyJson(root);
  }
  else {
    loadJson(root);
  }
  storeMutex.unlock();
}

string& MemEd::getNotes() {
  return notes;
}

void MemEd::setNotes(const string& notes) {
  this->notes = notes;
}

void MemEd::addNewAddress() {
  auto memio = scanner->getMemIO();
  SemPtr sem = SemPtr(new Sem(scanTypeToSize(ScanType::Int32), memio));
  sem->setScanType(SCAN_TYPE_INT_32);
  sem->setDescription("No description");
  store->getList().push_back(sem);
}

MemPtr MemEd::readMemory(Address addr, size_t size) {
  auto memio = scanner->getMemIO();
  return memio->read(addr, size);
}

void MemEd::setValueByAddress(Address addr, const string& value, const string& scanType) {
  auto memio = scanner->getMemIO();
  size_t size = scanTypeToSize(scanType);
  PemPtr pem = PemPtr(new Pem(addr, size, memio));
  pem->setValue(value, scanType);
}

void MemEd::setScopeStart(Address addr) {
  scanner->setScopeStart(addr);
}

void MemEd::setScopeEnd(Address addr) {
  scanner->setScopeEnd(addr);
}

std::mutex& MemEd::getScanListMutex() {
  return scanner->getListMutex();
}

void MemEd::resumeProcess() {
  isProcessPaused = false;
  if (pid && isPidSuspended(pid)) {
    pidResume(pid);
  }
}

void MemEd::pauseProcess() {
  if (!pid) return;

  isProcessPaused = true;
  pidStop(pid);
}

void MemEd::setCanResumeProcess(bool value) {
  canResumeProcess = value;
}

bool MemEd::getCanResumeProcess() {
  return canResumeProcess;
}

bool MemEd::getIsProcessPaused() {
  return isProcessPaused;
}
