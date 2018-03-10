#include "mem/MemEd.hpp"

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

vector<MemPtr> MemEd::scan(const string& value) {
  int v = stol(value);
  vector<MemPtr> mems = scanner->scan((Byte*)&v, 4, "int32", ScanParser::OpType::Eq);
  manager->setMems(mems);
  return mems;
}

vector<MemPtr> MemEd::filter(const string& value) {
  int v = stol(value);
  vector<MemPtr> mems = scanner->filter(manager->getMems(), (Byte*)&v, 4, "int32", ScanParser::OpType::Eq);
  manager->setMems(mems);
  return mems;
}

vector<MemPtr>& MemEd::getMems() {
  return manager->getMems();
}
