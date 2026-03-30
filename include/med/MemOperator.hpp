#ifndef MED_MEM_OPERATOR_HPP
#define MED_MEM_OPERATOR_HPP

#include <string>
#include "med/MedTypes.hpp"
#include "med/Operands.hpp"
#include "med/ScanParser.hpp"

namespace MemOperator {
    // Basic comparison for raw bytes
    bool compare(const void* ptr1, const void* ptr2, size_t size, ScanParser::OpType op);
    
    // Generic comparison for different ScanTypes
    bool compare(const void* ptr, ScanType type, const Operands& operands, ScanParser::OpType op);

    // Template based comparison for better performance and correctness
    template<typename T>
    bool compareTyped(T val1, T val2, ScanParser::OpType op) {
        switch (op) {
            case ScanParser::OpType::Eq: return val1 == val2;
            case ScanParser::OpType::Neq: return val1 != val2;
            case ScanParser::OpType::Gt: return val1 > val2;
            case ScanParser::OpType::Lt: return val1 < val2;
            case ScanParser::OpType::Ge: return val1 >= val2;
            case ScanParser::OpType::Le: return val1 <= val2;
            default: return false;
        }
    }

    template<typename T>
    bool withinTyped(T val, T low, T high) {
        return val >= low && val <= high;
    }

    std::string toString(const Byte* memory, ScanType type);
}

#endif
