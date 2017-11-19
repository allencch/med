//#define _FILE_OFFSET_BITS 64
#include <iostream>
#include <string>
#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <mutex>

#include <unistd.h> //getpagesize()

#include "med/MemOperator.hpp"
#include "med/med.hpp"

using namespace std;

const int STEP = 1;
std::mutex medMutex;

Med::Med() {
  threadManager = new ThreadManager(8);
  snapshot = new Snapshot();
}

Med::~Med() {
  clearStore();
  delete threadManager;
  delete snapshot;
}

void Med::scan(string v, string t) {
  if (!ScanParser::isValid(v)) {
    cerr << "Invalid scan string" << endl;
    return;
  }

  ScanParser::OpType op = ScanParser::getOpType(v);

  if (op == ScanParser::OpType::SnapshotSave) {
    // snapshot->save(&selectedProcess);
  }
  else {
    Bytes bytes = ScanParser::getBytes(v, t);

    Med::memScan(this, this->scanAddresses, stoi(selectedProcess.pid), bytes.data, bytes.size, t, op);

    delete[] bytes.data;
  }
}

void Med::filter(string v, string t) {
  if (!ScanParser::isValid(v)) {
    cerr << "Invalid scan string" << endl;
    return;
  }

  ScanParser::OpType op = ScanParser::getOpType(v);

  if (ScanParser::isSnapshotOperator(op) && !ScanParser::hasValues(v)) {
    // scanAddresses = snapshot->compare(op, stringToScanType(t));
  }
  else {
    Bytes bytes = ScanParser::getBytes(v, t);

    Med::memFilter(this->scanAddresses, stoi(this->selectedProcess.pid), bytes.data, bytes.size, t, op);
    delete[] bytes.data;
  }
}

void Med::memScanPage(Med* med,
                      uint8_t* page,
                      MemAddr start,
                      int srcSize,
                      vector<MedScan> &scanAddresses,
                      Byte* data,
                      int size,
                      string scanType,
                      ScanParser::OpType op) {
  //Once get the page, now can compare the data within the page
  for(int k = 0; k <= getpagesize() - size; k += STEP) {
    try {
      if(memCompare(page + k, srcSize, data, size, op)) {
        MedScan medScan(start + k);
        medScan.setScanType(scanType);
        med->scanAddressMutex.lock();
        scanAddresses.push_back(medScan);
        med->scanAddressMutex.unlock();
      }
    } catch(MedException& ex) {
      cerr << ex.getMessage() << endl;
    }
  }
}

void Med::memScanMap(Med* med,
                     ProcMaps& maps,
                     int mapIndex,
                     int fd,
                     int srcSize,
                     vector<MedScan>
                     &scanAddresses,
                     Byte* data,
                     int size,
                     string scanType,
                     ScanParser::OpType op) {
  for(MemAddr j = maps.starts[mapIndex]; j < maps.ends[mapIndex]; j += getpagesize()) {
    if(lseek(fd, j, SEEK_SET) == -1) {
      continue;
    }

    uint8_t* page = (uint8_t*)malloc(getpagesize()); //For block of memory

    if(read(fd, page, getpagesize()) == -1) {
      continue;
    }
    Med::memScanPage(med, page, j, srcSize, scanAddresses, data, size, scanType, op);

    free(page);
  }
}

void Med::memScan(Med* med, vector<MedScan> &scanAddresses, pid_t pid, Byte* data, int size, string scanType, ScanParser::OpType op) {
  medMutex.lock();
  pidAttach(pid);

  ProcMaps maps = getMaps(pid);
  int memFd = getMem(pid);
  int srcSize = size; // TODO: no need srcSize

  scanAddresses.clear();

  for (int i = 0; i < (int)maps.starts.size(); i++) {
    TMTask* fn = new TMTask();
    *fn = [med, &maps, i, memFd, srcSize, &scanAddresses, data, size, scanType, op]() {
      Med::memScanMap(med, maps, i, memFd, srcSize, scanAddresses, data, size, scanType, op);
    };
    med->threadManager->queueTask(fn);
  }

  med->threadManager->start();
  med->threadManager->clear();

  printf("Found %lu results\n",scanAddresses.size());
  close(memFd);

  pidDetach(pid);
  if (med->scanAddresses.size() < SCAN_ADDRESS_VISIBLE_SIZE) {
    med->sortScanByAddress();
  }
  medMutex.unlock();
}

void Med::memFilter(vector<MedScan> &scanAddresses, pid_t pid, Byte* data, int size, string scanType, ScanParser::OpType op) {
  medMutex.lock();
  pidAttach(pid);
  vector<MedScan> addresses; //New addresses

  int memFd = getMem(pid);

  int srcSize = size; // TODO: no need srcSize

  uint8_t* buf = (uint8_t*)malloc(size);
  for(unsigned int i=0;i<scanAddresses.size();i++) {
    if(lseek(memFd, scanAddresses[i].address, SEEK_SET) == -1) {
      continue;
    }
    if(read(memFd, buf, size) == -1) {
      continue;
    }

    if(memCompare(buf, srcSize, data, size, op)) {
      scanAddresses[i].setScanType(scanType);
      addresses.push_back(scanAddresses[i]);
    }
  }

  free(buf);

  //Remove old one
  scanAddresses = addresses;
  printf("Filtered %lu results\n",scanAddresses.size());
  close(memFd);
  pidDetach(pid);
  medMutex.unlock();
}

vector<Process> Med::listProcesses() {
  this->processes = pidList();
  return this->processes;
}

string Med::getScanAddressValueByIndex(int ind, string scanType) {
  return this->scanAddresses[ind].getValue(stol(this->selectedProcess.pid), scanType);
}

string Med::getScanValueByIndex(int ind) {
  return this->scanAddresses[ind].getValue(stol(this->selectedProcess.pid), this->scanAddresses[ind].getScanType());
}

string Med::getScanTypeByIndex(int ind) {
  return this->scanAddresses[ind].getScanType();
}

string Med::getStoreValueByIndex(int ind) {
  return this->addresses[ind]->getValue(stol(this->selectedProcess.pid), this->addresses[ind]->getScanType());
}

string Med::getValueByAddress(MemAddr address, string scanType) {
  string value;
  value = memValue(stol(this->selectedProcess.pid), address, scanType);
  return value;
}

void Med::setValueByAddress(MemAddr address, string value, string scanType) {
  uint8_t* buffer = NULL;
  int size = stringToRaw(string(value), scanType, &buffer);
  memWrite(stoi(this->selectedProcess.pid), address, buffer, size);
  if (buffer) {
    delete[] buffer;
  }
}

bool Med::addToStoreByIndex(int index) {
  if (index < 0 || index > (int)this->scanAddresses.size() - 1) {
    return false;
  }

  MedAddress* medAddress = new MedAddress();
  medAddress->description = "Your description";
  medAddress->address = this->scanAddresses[index].address;
  medAddress->scanType = this->scanAddresses[index].scanType;
  this->addresses.push_back(medAddress);

  return true;
}

void Med::saveFile(const char* filename) {
  Json::Value root;
  Json::Value addresses;
  root["addresses"] = addresses;

  for (auto address: this->addresses) {
    Json::Value pairs;
    pairs["description"] = address->description;
    pairs["address"] = intToHex(address->address);
    pairs["type"] = address->getScanType();
    try {
      pairs["value"] = string(address->getValue(stol(this->selectedProcess.pid), address->getScanType()));
    } catch(MedException &ex) {
      pairs["value"] = "";
    }
    pairs["lock"] = address->lock;
    root["addresses"].append(pairs);
  }
  root["notes"] = notes;

  ofstream ofs;
  ofs.open(filename);
  if (ofs.fail()) {
    throw MedException(string("Save JSON: Fail to open file ") + filename);
  }
  ofs << root << endl;
  ofs.close();
}

void Med::loadLegacyJson(Json::Value& root) {
  for (int i = 0; i < (int)root.size(); i++) {
    MedAddress* address = new MedAddress();
    address->description = root[i]["description"].asString();
    address->address = hexToInt(root[i]["address"].asString());
    address->setScanType(root[i]["type"].asString());
    address->lock = false; // always open as false, so that do not update the value

    this->addresses.push_back(address);
  }
}

void Med::loadJson(Json::Value& root) {
  auto& addresses = root["addresses"];
  for (int i = 0; i < (int)addresses.size(); i++) {
    MedAddress* address = new MedAddress();
    address->description = addresses[i]["description"].asString();
    address->address = hexToInt(addresses[i]["address"].asString());
    address->setScanType(addresses[i]["type"].asString());
    address->lock = false; // always open as false, so that do not update the value

    this->addresses.push_back(address);
  }
  notes = root["notes"].asString();
}

void Med::openFile(const char* filename) {
  Json::Value root;

  ifstream ifs;
  ifs.open(filename);
  if (ifs.fail()) {
    throw MedException(string("Open JSON: Fail to open file ") + filename);
  }
  ifs >> root;
  ifs.close();

  clearStore();
  if (root.isArray()) {
    loadLegacyJson(root);
  }
  else {
    loadJson(root);
  }
}

void Med::lockAddressValueByIndex(int ind) {
  //Create a thread
  MedAddress* address = this->addresses[ind];
  if (address->lock)
    return;
  address->lockValue(this->selectedProcess.pid);
}

void Med::unlockAddressValueByIndex(int ind) {
  MedAddress* address = this->addresses[ind];
  if (!address->lock)
    return;
  address->unlockValue();
}

void Med::setStoreLockByIndex(int ind, bool lockStatus) {
  lockStatus ? lockAddressValueByIndex(ind) : unlockAddressValueByIndex(ind);
}

bool Med::getStoreLockByIndex(int ind) {
  return addresses[ind]->lock;
}

MedAddress* Med::addNewAddress() {
  MedAddress* medAddress = new MedAddress();
  medAddress->description = "Your description";
  medAddress->address = 0;
  medAddress->setScanType("int16");
  medAddress->lock = false;
  addresses.push_back(medAddress);
  return medAddress;
}

MedAddress* Med::duplicateAddress(int ind) {
  if (ind < 0 || ind >= (int)addresses.size()) {
    cerr << "Index out of range" << endl;
    return NULL;
  }
  MedAddress* addr = addresses[ind];

  MedAddress* newAddr = addNewAddress();
  newAddr->address = addr->address;
  newAddr->setScanType(addr->getScanType());
  newAddr->lock = false;
  return newAddr;
}

MedAddress* Med::addNextAddress(int ind) {
  MedAddress* newAddr = duplicateAddress(ind);
  if (!newAddr) {
    return NULL;
  }
  int step = scanTypeToSize(newAddr->getScanType());
  newAddr->address = newAddr->address + step;
  return newAddr;
}

MedAddress* Med::addPrevAddress(int ind) {
  MedAddress* newAddr = duplicateAddress(ind);
  if (!newAddr) {
    return NULL;
  }
  int step = scanTypeToSize(newAddr->getScanType());
  newAddr->address = newAddr->address - step;
  return newAddr;
}


string Med::getScanAddressByIndex(int ind) {
  char address[32];
  sprintf(address, "%p", (void*)(scanAddresses[ind].address));
  return string(address);
}

string Med::getStoreAddressByIndex(int ind) {
  char address[32];
  sprintf(address, "%p", (void*)(addresses[ind]->address));
  return string(address);
}

void Med::deleteAddressByIndex(int ind) {
  delete addresses[ind];
  addresses.erase(addresses.begin() + ind);
}

void Med::shiftStoreAddresses(long diff) {
  for (unsigned int i=0; i < addresses.size(); i++) {
    shiftStoreAddressByIndex(i, diff);
  }
}

void Med::shiftStoreAddressByIndex(int ind, long diff) {
  long address = addresses[ind]->address;
  address += diff;
  addresses[ind]->address = address;
}

string Med::getStoreDescriptionByIndex(int ind) {
  return addresses[ind]->description;
}

string Med::setStoreAddressByIndex(int ind, string address) {
  addresses[ind]->address = hexToInt(address);
  string value = getValueByAddress(addresses[ind]->address, addresses[ind]->getScanType());
  return value;
}

void Med::clearStore() {
  for(unsigned int i=0;i<addresses.size();i++)
    delete addresses[i];
  addresses.clear();
}

void Med::clearScan() {
  scanAddresses.clear();
}

void Med::setStoreDescriptionByIndex(int ind, string description) {
  addresses[ind]->description = description;
}

void Med::sortStoreByAddress() {
  sort(addresses.begin(), addresses.end(), [](MedAddress* a, MedAddress* b) {
      return a->address < b->address;
    });
}

void Med::sortStoreByDescription() {
  sort(addresses.begin(), addresses.end(), [](MedAddress* a, MedAddress* b) {
      return a->description.compare(b->description) < 0;
    });
}

void Med::sortScanByAddress() {
  sort(scanAddresses.begin(), scanAddresses.end(), [](MedScan a, MedScan b) {
      return a.address < b.address;
    });
}

Byte* Med::readMemory(MemAddr address, size_t size) {
  return memRead(stol(this->selectedProcess.pid), address, size);
}
