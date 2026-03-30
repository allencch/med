#ifndef MED_OPERANDS_HPP
#define MED_OPERANDS_HPP

#include <vector>
#include "med/MedTypes.hpp"
#include "med/SizedBytes.hpp"

class Operands {
public:
    Operands() = default;
    explicit Operands(std::vector<SizedBytes> data);

    size_t count() const;
    const SizedBytes& getFirstOperand() const;
    const SizedBytes& getSecondOperand() const;
    size_t getFirstSize() const;

private:
    std::vector<SizedBytes> data_;
};

#endif
