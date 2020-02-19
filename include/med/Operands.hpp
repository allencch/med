#ifndef OPERANDS_HPP
#define OPERANDS_HPP

#include "med/MedTypes.hpp"

class Operands {
public:
  Operands();
  explicit Operands(std::vector<SizedBytes> l);
  size_t count();

  SizedBytes getFirstOperand();
  SizedBytes getSecondOperand();
private:
  std::vector<SizedBytes> data;
};

#endif
