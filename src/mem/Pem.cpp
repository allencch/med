#include <cinttypes>
#include <cstring>
#include <cstdio>
#include <iostream>

#include "mem/Pem.hpp"
#include "med/MedCommon.hpp"
#include "med/MedException.hpp"
#include "med/ScanParser.hpp"
#include "med/MedTypes.hpp"

Pem::Pem(size_t size, MemIO* memio) : Mem(size) {
  this->memio = memio;
  rememberedValue = NULL;
  rememberedSize = 0;
  scanType = ScanType::Unknown;
}

Pem::Pem(Address addr, size_t size, MemIO* memio) : Mem(size) {
  this->memio = memio;
  rememberedValue = NULL;
  rememberedSize = 0;
  setAddress(addr);
  scanType = ScanType::Unknown;
}

Pem::~Pem() {
  if (rememberedValue) {
    delete[] rememberedValue;
  }
}

string Pem::bytesToString(Byte* buf, const string& scanType) {
  char str[MAX_STRING_SIZE];
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
    throw_with_nested(MedException(string("bytesToString: Error Type: ") + scanType));
  }
  return string(str);
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

tuple<Byte*, size_t> Pem::stringToBytes(const string& value, const string& scanType) {
  Byte* buf;
  int size;

  // If scanType is string, it will just compy  all
  if (scanType == SCAN_TYPE_STRING) {
    size = MAX_STRING_SIZE;
    buf = new Byte[size];
    sprintf((char*)buf, "%s", value.c_str());
    int length = strlen((char*)buf);

    return make_tuple(buf, length);
  }
  else { // Allows parse comma
    size = scanTypeToSize(stringToScanType(scanType));
    vector<string> tokens = ScanParser::getValues(value);
    buf = new Byte[size * tokens.size()];

    Byte* ptr = buf;
    for (size_t i = 0; i < tokens.size(); i++) {
      stringToMemory(tokens[i], stringToScanType(scanType), ptr);
      ptr += size;
    }

    return make_tuple(buf, size * tokens.size()); // delete
  }
}

void Pem::setValue(const string& value, const string& scanType) {
  auto buffer = Pem::stringToBytes(value, scanType);
  Byte* bytes = std::get<0>(buffer);
  int size = std::get<1>(buffer);

  MemPtr mem(new Mem(size));
  mem->setAddress(address);
  memcpy(mem->getData(), bytes, size);
  delete[] bytes;

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
  auto bytes = Pem::stringToBytes(value, scanType);
  if (rememberedValue) {
    delete[] rememberedValue;
  }
  rememberedValue = std::get<0>(bytes);
  rememberedSize = std::get<1>(bytes);
}

void Pem::rememberValue(Byte* value, size_t size) {
  if (rememberedValue) {
    delete[] rememberedValue;
  }
  rememberedValue = new Byte[size];
  rememberedSize = size;
  memcpy(rememberedValue, value, size);
}

string Pem::recallValue(const string& scanType) {
  if (rememberedValue == NULL) {
    return "";
  }
  return Pem::bytesToString(rememberedValue, scanType);
}

Byte* Pem::recallValuePtr() {
  return rememberedValue;
}

PemPtr Pem::convertToPemPtr(MemPtr mem, MemIO* memio) {
  return PemPtr(new Pem(mem->getAddress(), mem->getSize(), memio));
}
