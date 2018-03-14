#include "mem/MemEd.hpp"
#include "med/MedCommon.hpp"
#include "mem/Sem.hpp"

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
}

void MemEd::initialize() {
  pid = 0;
  scanner = new MemScanner();
  manager = new MemManager();

  vector<MemPtr> emptyMems;
  store = new MemList(emptyMems);

  lockValueThread = new std::thread(MemEd::callLockValue, this);
}

void MemEd::setPid(pid_t pid) {
  this->pid = pid;
  scanner->setPid(pid);
}

pid_t MemEd::getPid() {
  return pid;
}

vector<MemPtr> MemEd::scan(const string& value, const string& scanType) {
  auto buffer = ScanParser::valueToBytes(value, scanType);
  size_t size = std::get<1>(buffer);

  vector<MemPtr> mems = scanner->scan(std::get<0>(buffer), size, scanType, ScanParser::OpType::Eq);
  manager->setMems(mems);

  delete[] std::get<0>(buffer);
  return mems;
}

vector<MemPtr> MemEd::filter(const string& value, const string& scanType) {
  auto buffer = ScanParser::valueToBytes(value, scanType);
  size_t size = std::get<1>(buffer);

  vector<MemPtr> mems = scanner->filter(manager->getMems(), std::get<0>(buffer), size, scanType, ScanParser::OpType::Eq);
  manager->setMems(mems);

  delete[] std::get<0>(buffer);
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
  MemPtr sem = MemPtr(new Sem(pem));
  getStore()->addMemPtr(sem);
}

void MemEd::callLockValue(MemEd* med) {
  med->lockValue();
}

void MemEd::lockValue() {
  storeMutex.lock();

  storeMutex.unlock();
}
