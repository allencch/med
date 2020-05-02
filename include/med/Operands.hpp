#ifndef OPERANDS_HPP
#define OPERANDS_HPP

#include "med/MedTypes.hpp"
#include "med/SizedBytes.hpp"

class Operands {
public:
  Operands();
  explicit Operands(std::vector<SizedBytes> l);
  size_t count();

  SizedBytes getFirstOperand();
  SizedBytes getSecondOperand();
  ScanParser::OpType getOperator();

  size_t getFirstSize();
private:
  std::vector<SizedBytes> data;
  ScanParser::OpType op;
};

#endif
