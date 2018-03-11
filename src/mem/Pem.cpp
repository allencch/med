#include <cinttypes>
#include <cstring>
#include <cstdio>

#include "mem/Pem.hpp"
#include "med/MedCommon.hpp"
#include "med/MedException.hpp"

Pem::Pem(size_t size, MemIO* memio) : Mem(size) {
  this->memio = memio;
}

Pem::Pem(Address addr, size_t size, MemIO* memio) : Mem(size) {
  this->memio = memio;
  setAddress(addr);
}

string Pem::bytesToString(Byte* buf, const string& scanType) {
  char str[MAX_STRING_SIZE + 1];
  switch (stringToScanType(scanType)) {
  case Int8:
    sprintf(str, "%" PRIu8, *(uint8_t*)buf);
    break;
  case Int16:
    sprintf(str, "%" PRIu16, *(uint16_t*)buf);
    break;
  case Int32:
    sprintf(str, "%" PRIu32, *(uint32_t*)buf);
    break;
  case Float32:
    sprintf(str, "%f", *(float*)buf);
    break;
  case Float64:
    sprintf(str, "%lf", *(double*)buf);
    break;
  case String:
    sprintf(str, "%s", buf);
    break;
  case Unknown:
    throw MedException(string("memValue: Error Type: ") + scanType);
  }
  return string(str);
}

string Pem::getValue(const string& scanType) {
  MemPtr pem = memio->read(address, size);
  Byte* buf = pem->data;
  return Pem::bytesToString(buf, scanType);
}

string Pem::getScanType() {
  return scanTypeToString(scanType);
}

tuple<Byte*, size_t> Pem::stringToBytes(const string& value, const string& scanType) {
  int size = scanTypeToSize(stringToScanType(scanType));
  vector<string> tokens = ScanParser::getValues(value);
  Byte* buf = new Byte[size * tokens.size()];

  Byte* ptr = buf;
  for (size_t i = 0; i < tokens.size(); i++) {
    stringToMemory(tokens[i], stringToScanType(scanType), ptr);
    ptr += size;
  }
  return make_tuple(buf, size * tokens.size()); // delete
}

void Pem::setValue(const string& value, const string& scanType) {
  auto buffer = Pem::stringToBytes(value, scanType);
  Byte* bytes = std::get<0>(buffer);
  int size = std::get<1>(buffer);

  MemPtr mem = MemPtr(new Mem(size));
  mem->setAddress(address);
  memcpy(mem->data, bytes, size);
  delete[] bytes;

  memio->write(address, mem, size);
}

void Pem::setScanType(const string& scanType) {
  this->scanType = stringToScanType(scanType);
}
