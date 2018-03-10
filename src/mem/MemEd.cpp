#include "mem/MemEd.hpp"
#include "med/MedCommon.hpp"

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
}

void MemEd::initialize() {
  pid = 0;
  scanner = new MemScanner();
  manager = new MemManager();
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

vector<MemPtr>& MemEd::getMems() {
  return manager->getMems();
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
