#include <vector>
#include "med/Operands.hpp"
#include "med/MedException.hpp"

Operands::Operands() {
  op = ScanParser::OpType::Eq;
}
Operands::Operands(std::vector<SizedBytes> l) : data(l) {
  op = ScanParser::OpType::Eq;
}

size_t Operands::count() {
  return data.size();
}

SizedBytes Operands::getFirstOperand() {
  auto size = count();
  if (size < 1) {
    throw MedException("Operands size should not less than 1");
  }
  return data[0];
};

SizedBytes Operands::getSecondOperand() {
  auto size = count();
  if (size < 2) {
    throw MedException("Operands size should not less than 2");
  }
  return data[1];
}

size_t Operands::getFirstSize() {
  return data[0].getSize();
}

ScanParser::OpType Operands::getOperator() {
  return op;
}
