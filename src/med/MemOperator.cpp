#include <cstring>
#include <cstdio>
#include <cinttypes>
#include "med/MemOperator.hpp"
#include "med/MedException.hpp"
#include "med/MedCommon.hpp"
#include "med/Coder.hpp"

namespace MemOperator {

bool compare(const void* ptr1, const void* ptr2, size_t size, ScanParser::OpType op) {
    int res = std::memcmp(ptr1, ptr2, size);
    switch (op) {
        case ScanParser::OpType::Eq: return res == 0;
        case ScanParser::OpType::Neq: return res != 0;
        // For multi-byte comparison like Gt/Lt, memcmp isn't enough for numerical
        // This function is mostly for strings/bytes where Eq/Neq is what matters
        default: return res == 0; 
    }
}

template<typename T>
bool compareTypedDispatch(const void* ptr, const Operands& operands, ScanParser::OpType op) {
    if (op == ScanParser::OpType::Within || op == ScanParser::OpType::Around) {
        T val = *reinterpret_cast<const T*>(ptr);
        T low = *reinterpret_cast<const T*>(operands.getFirstOperand().getBytes());
        T high = *reinterpret_cast<const T*>(operands.getSecondOperand().getBytes());
        return withinTyped(val, low, high);
    } else {
        const Byte* bytePtr = reinterpret_cast<const Byte*>(ptr);
        for (size_t i = 0; i < operands.count(); ++i) {
            T val = *reinterpret_cast<const T*>(bytePtr + i * sizeof(T));
            T opVal = *reinterpret_cast<const T*>(operands.getOperand(i).getBytes());
            if (!compareTyped(val, opVal, op)) return false;
        }
        return true;
    }
}

template<typename T>
bool compareTypedPointers(const void* ptr1, const void* ptr2, ScanParser::OpType op) {
    T val1 = *reinterpret_cast<const T*>(ptr1);
    T val2 = *reinterpret_cast<const T*>(ptr2);
    return compareTyped(val1, val2, op);
}

bool compare(const void* ptr, ScanType type, const Operands& operands, ScanParser::OpType op) {
    switch (type) {
        case ScanType::Int8: return compareTypedDispatch<int8_t>(ptr, operands, op);
        case ScanType::Int16: return compareTypedDispatch<int16_t>(ptr, operands, op);
        case ScanType::Int32: return compareTypedDispatch<int32_t>(ptr, operands, op);
        case ScanType::Int64: return compareTypedDispatch<int64_t>(ptr, operands, op);
        case ScanType::UInt8: return compareTypedDispatch<uint8_t>(ptr, operands, op);
        case ScanType::UInt16: return compareTypedDispatch<uint16_t>(ptr, operands, op);
        case ScanType::UInt32: return compareTypedDispatch<uint32_t>(ptr, operands, op);
        case ScanType::UInt64: return compareTypedDispatch<uint64_t>(ptr, operands, op);
        case ScanType::Float32: return compareTypedDispatch<float>(ptr, operands, op);
        case ScanType::Float64: return compareTypedDispatch<double>(ptr, operands, op);
        case ScanType::Ptr32: return compareTypedDispatch<uint32_t>(ptr, operands, op);
        case ScanType::Ptr64: return compareTypedDispatch<uint64_t>(ptr, operands, op);
        case ScanType::String: {
            const Byte* bytePtr = reinterpret_cast<const Byte*>(ptr);
            size_t offset = 0;
            for (size_t i = 0; i < operands.count(); ++i) {
                const SizedBytes& opBytes = operands.getOperand(i);
                if (!compare(bytePtr + offset, opBytes.getBytes(), opBytes.getSize(), op)) return false;
                offset += opBytes.getSize();
            }
            return true;
        }
        default: return false;
    }
}

bool compare(const void* ptr1, const void* ptr2, ScanType type, ScanParser::OpType op) {
    switch (type) {
        case ScanType::Int8: return compareTypedPointers<int8_t>(ptr1, ptr2, op);
        case ScanType::Int16: return compareTypedPointers<int16_t>(ptr1, ptr2, op);
        case ScanType::Int32: return compareTypedPointers<int32_t>(ptr1, ptr2, op);
        case ScanType::Int64: return compareTypedPointers<int64_t>(ptr1, ptr2, op);
        case ScanType::UInt8: return compareTypedPointers<uint8_t>(ptr1, ptr2, op);
        case ScanType::UInt16: return compareTypedPointers<uint16_t>(ptr1, ptr2, op);
        case ScanType::UInt32: return compareTypedPointers<uint32_t>(ptr1, ptr2, op);
        case ScanType::UInt64: return compareTypedPointers<uint64_t>(ptr1, ptr2, op);
        case ScanType::Float32: return compareTypedPointers<float>(ptr1, ptr2, op);
        case ScanType::Float64: return compareTypedPointers<double>(ptr1, ptr2, op);
        case ScanType::Ptr32: return compareTypedPointers<uint32_t>(ptr1, ptr2, op);
        case ScanType::Ptr64: return compareTypedPointers<uint64_t>(ptr1, ptr2, op);
        case ScanType::String: {
            if (op == ScanParser::OpType::Eq) {
                return std::strncmp((const char*)ptr1, (const char*)ptr2, MAX_STRING_SIZE) == 0;
            } else if (op == ScanParser::OpType::Neq) {
                return std::strncmp((const char*)ptr1, (const char*)ptr2, MAX_STRING_SIZE) != 0;
            }
            return false;
        }
        default: return false;
    }
}

std::string toString(const Byte* memory, ScanType type, EncodingType encoding) {
    return toString(memory, MedUtil::scanTypeToSize(type), type, encoding);
}

std::string toString(const Byte* memory, size_t size, ScanType type, EncodingType encoding) {
    char str[MAX_STRING_SIZE];
    switch (type) {
        case ScanType::Int8: sprintf(str, "%" PRId8, *(const int8_t*)memory); break;
        case ScanType::Int16: sprintf(str, "%" PRId16, *(const int16_t*)memory); break;
        case ScanType::Int32: sprintf(str, "%" PRId32, *(const int32_t*)memory); break;
        case ScanType::Int64: sprintf(str, "%" PRId64, *(const int64_t*)memory); break;
        case ScanType::UInt8: sprintf(str, "%" PRIu8, *(const uint8_t*)memory); break;
        case ScanType::UInt16: sprintf(str, "%" PRIu16, *(const uint16_t*)memory); break;
        case ScanType::UInt32: sprintf(str, "%" PRIu32, *(const uint32_t*)memory); break;
        case ScanType::UInt64: sprintf(str, "%" PRIu64, *(const uint64_t*)memory); break;
        case ScanType::Ptr32: sprintf(str, "0x%" PRIx32, *(const uint32_t*)memory); break;
        case ScanType::Ptr64: sprintf(str, "0x%" PRIx64, *(const uint64_t*)memory); break;
        case ScanType::Float32: sprintf(str, "%f", *(const float*)memory); break;
        case ScanType::Float64: sprintf(str, "%lf", *(const double*)memory); break;
        case ScanType::String: {
            std::string text = std::string((const char*)memory);
            if (encoding == EncodingType::Big5) {
                return convertBig5ToUtf8(text);
            }
            return text;
        }
        case ScanType::Custom: {
            std::string hex;
            for (size_t i = 0; i < std::min(size, (size_t)16); ++i) {
                char buf[4];
                sprintf(buf, "%02X ", memory[i]);
                hex += buf;
            }
            if (size > 16) hex += "...";
            return hex;
        }
        default: return "Unknown";
    }
    return std::string(str);
}

} // namespace MemOperator
