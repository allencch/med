#include "med/Operands.hpp"
#include "med/MedException.hpp"

Operands::Operands(std::vector<SizedBytes> data) : data_(std::move(data)) {}

size_t Operands::count() const {
    return data_.size();
}

const SizedBytes& Operands::getFirstOperand() const {
    if (data_.empty()) {
        throw MedException("No operands available");
    }
    return data_[0];
}

const SizedBytes& Operands::getSecondOperand() const {
    if (data_.size() < 2) {
        throw MedException("Second operand not available");
    }
    return data_[1];
}

size_t Operands::getFirstSize() const {
    if (data_.empty()) return 0;
    return data_[0].getSize();
}
