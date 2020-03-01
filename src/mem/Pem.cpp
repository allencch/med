#include <cinttypes>
#include <cstring>
#include <cstdio>
#include <iostream>

#include "mem/Pem.hpp"
#include "med/MedCommon.hpp"
#include "med/MedException.hpp"
#include "med/ScanParser.hpp"
#include "med/MedTypes.hpp"
#include "med/MemOperator.hpp"

Pem::Pem(size_t size, MemIO* memio) : Mem(size) {
  this->memio = memio;
  scanType = ScanType::Unknown;
}

Pem::Pem(Address addr, size_t size, MemIO* memio) : Mem(size) {
  this->memio = memio;
  setAddress(addr);
  scanType = ScanType::Unknown;
}

Pem::~Pem() {}

string Pem::bytesToString(Byte* buf, const string& scanType) {
  return memToString(buf, scanType);
}

string Pem::getValue(const string& scanType) {
  MemPtr pem = memio->read(address, size);
  if (!pem) {
    return "(invalid)";
  }
  Byte* buf = pem->getData();
  return Pem::bytesToString(buf, scanType);
}

string Pem::getValue() {
  string scanType = getScanType();
  return getValue(scanType);
}

BytePtr Pem::getValuePtr(int n) {
  int size = n > 0 ? n : this->size;
  BytePtr buf(new Byte[size]);
  MemPtr pem = memio->read(address, size);
  if (!pem) {
    return NULL;
  }
  memcpy(buf.get(), pem->getData(), size);
  return buf;
}

string Pem::getScanType() {
  return scanTypeToString(scanType);
}

SizedBytes Pem::stringToBytes(const string& value, const string& scanType) {
  // If scanType is string, it will just copy all
  if (scanType == SCAN_TYPE_STRING) {
    int size = MAX_STRING_SIZE;
    Byte* buf = new Byte[size];
    sprintf((char*)buf, "%s", value.c_str());
    int length = strlen((char*)buf);

    BytePtr data(new Byte[length]);
    memcpy(data.get(), buf, length);
    delete[] buf;

    return SizedBytes(data, length);
  }
  else { // Allows parse comma
    vector<string> tokens = ScanParser::getValues(value);
    int size = scanTypeToSize(stringToScanType(scanType));
    BytePtr data(new Byte[size * tokens.size()]);
    Byte* pointer = data.get();

    for (size_t i = 0; i < tokens.size(); i++) {
      stringToMemory(tokens[i], stringToScanType(scanType), pointer);
      pointer += size;
    }

    return SizedBytes(data, size * tokens.size());
  }
}

void Pem::setValue(const string& value, const string& scanType) {
  SizedBytes buffer = Pem::stringToBytes(value, scanType);
  Byte* bytes = buffer.getBytes();
  int size = buffer.getSize();

  MemPtr mem(new Mem(size));
  mem->setAddress(address);
  memcpy(mem->getData(), bytes, size);

  memio->write(address, mem, size);
}

void Pem::setScanType(const string& scanType) {
  this->scanType = stringToScanType(scanType);
  size_t newSize;
  Byte* newData;
  Byte* temp;

  if (this->scanType == ScanType::String) {
    newSize = MAX_STRING_SIZE;
    newData = new Byte[newSize];
  }
  else {
    newSize = scanTypeToSize(this->scanType);
    newData = new Byte[newSize];
  }

  temp = data;
  data = newData;
  size = newSize;
  delete[] temp;
}

MemIO* Pem::getMemIO() {
  return memio;
}

void Pem::rememberValue(const string& value, const string& scanType) {
  rememberedValue = Pem::stringToBytes(value, scanType);
}

void Pem::rememberValue(Byte* value, size_t size) {
  rememberedValue = SizedBytes(value, size);
}

string Pem::recallValue(const string& scanType) {
  if (rememberedValue.isEmpty()) {
    return "";
  }
  return Pem::bytesToString(rememberedValue.getBytes(), scanType);
}

Byte* Pem::recallValuePtr() {
  return rememberedValue.getBytes();
}

PemPtr Pem::convertToPemPtr(MemPtr mem, MemIO* memio) {
  return PemPtr(new Pem(mem->getAddress(), mem->getSize(), memio));
}
