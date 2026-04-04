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

const SizedBytes& Operands::getOperand(size_t index) const {
    if (index >= data_.size()) {
        throw MedException("Operand index out of bounds");
    }
    return data_[index];
}

size_t Operands::getFirstSize() const {
    if (data_.empty()) return 0;
    return data_[0].getSize();
}

size_t Operands::getTotalSize() const {
    size_t total = 0;
    for (const auto& sb : data_) {
        total += sb.getSize();
    }
    return total;
}
