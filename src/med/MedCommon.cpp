#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>

#include "med/MedCommon.hpp"
#include "med/MedException.hpp"
#include "med/Coder.hpp"

namespace MedUtil {

ScanType stringToScanType(const std::string& scanType) {
    if (scanType == ScanTypeString::Int8) return ScanType::UInt8;
    if (scanType == ScanTypeString::Int16) return ScanType::UInt16;
    if (scanType == ScanTypeString::Int32) return ScanType::UInt32;
    if (scanType == ScanTypeString::Int64) return ScanType::UInt64;
    if (scanType == ScanTypeString::UInt8) return ScanType::UInt8;
    if (scanType == ScanTypeString::UInt16) return ScanType::UInt16;
    if (scanType == ScanTypeString::UInt32) return ScanType::UInt32;
    if (scanType == ScanTypeString::UInt64) return ScanType::UInt64;
    if (scanType == ScanTypeString::Float32) return ScanType::Float32;
    if (scanType == ScanTypeString::Float64) return ScanType::Float64;
    if (scanType == ScanTypeString::String) return ScanType::String;
    if (scanType == ScanTypeString::Custom) return ScanType::Custom;
    if (scanType == ScanTypeString::Ptr32) return ScanType::Ptr32;
    if (scanType == ScanTypeString::Ptr64) return ScanType::Ptr64;
    return ScanType::Unknown;
}

std::string scanTypeToString(const ScanType& scanType) {
    switch (scanType) {
        case ScanType::Int8: return ScanTypeString::Int8;
        case ScanType::Int16: return ScanTypeString::Int16;
        case ScanType::Int32: return ScanTypeString::Int32;
        case ScanType::Int64: return ScanTypeString::Int64;
        case ScanType::UInt8: return ScanTypeString::Int8;
        case ScanType::UInt16: return ScanTypeString::Int16;
        case ScanType::UInt32: return ScanTypeString::Int32;
        case ScanType::UInt64: return ScanTypeString::Int64;
        case ScanType::Float32: return ScanTypeString::Float32;
        case ScanType::Float64: return ScanTypeString::Float64;
        case ScanType::String: return ScanTypeString::String;
        case ScanType::Custom: return ScanTypeString::Custom;
        case ScanType::Ptr32: return ScanTypeString::Ptr32;
        case ScanType::Ptr64: return ScanTypeString::Ptr64;
        default: return ScanTypeString::Unknown;
    }
}

size_t scanTypeToSize(const ScanType& type) {
    switch (type) {
        case ScanType::Int8:
        case ScanType::UInt8: return sizeof(uint8_t);
        case ScanType::Int16:
        case ScanType::UInt16: return sizeof(uint16_t);
        case ScanType::Int32:
        case ScanType::UInt32:
        case ScanType::Ptr32: return sizeof(uint32_t);
        case ScanType::Int64:
        case ScanType::UInt64:
        case ScanType::Ptr64: return sizeof(uint64_t);
        case ScanType::Float32: return sizeof(float);
        case ScanType::Float64: return sizeof(double);
        case ScanType::String: return MAX_STRING_SIZE;
        default: return 0;
    }
}

std::string intToHex(Address addr) {
    std::stringstream ss;
    ss << "0x" << std::hex << addr;
    return ss.str();
}

Address hexToInt(const std::string& str) {
    Address ret = 0;
    std::stringstream ss;
    if (str.find("0x") == 0 || str.find("0X") == 0) {
        ss << std::hex << str.substr(2);
    } else {
        ss << std::hex << str;
    }
    ss >> ret;
    if (ss.fail()) {
        throw MedException("Invalid hexadecimal string: " + str);
    }
    return ret;
}

Address addressRoundDown(Address addr) {
    return addr - (addr % 16);
}

void stringToMemory(const std::string& str, ScanType type, Byte* buffer, EncodingType encoding) {
    if (isHexString(str)) {
        return hexStringToMemory(str, type, buffer);
    }

    if (type == ScanType::String) {
        std::string encodedValue = str;
        if (encoding == EncodingType::Big5) {
            encodedValue = convertFromUtf8(str, "big5");
        }
        std::memcpy(buffer, encodedValue.c_str(), std::min(encodedValue.size() + 1, (size_t)MAX_STRING_SIZE));
        return;
    }

    std::stringstream ss(str);
    switch (type) {
        case ScanType::Int8: {
            int temp;
            ss >> std::dec >> temp;
            *(int8_t*)buffer = (int8_t)temp;
            break;
        }
        case ScanType::UInt8: {
            uint32_t temp;
            ss >> std::dec >> temp;
            *(uint8_t*)buffer = (uint8_t)temp;
            break;
        }
        case ScanType::Int16: {
            int16_t temp;
            ss >> std::dec >> temp;
            *(int16_t*)buffer = temp;
            break;
        }
        case ScanType::UInt16: {
            uint16_t temp;
            ss >> std::dec >> temp;
            *(uint16_t*)buffer = temp;
            break;
        }
        case ScanType::Int32:
        case ScanType::UInt32:
        case ScanType::Ptr32: {
            uint32_t temp;
            ss >> std::dec >> temp;
            *(uint32_t*)buffer = temp;
            break;
        }
        case ScanType::Int64:
        case ScanType::UInt64:
        case ScanType::Ptr64: {
            uint64_t temp;
            ss >> std::dec >> temp;
            *(uint64_t*)buffer = temp;
            break;
        }
        case ScanType::Float32: {
            float temp;
            ss >> std::dec >> temp;
            *(float*)buffer = temp;
            break;
        }
        case ScanType::Float64: {
            double temp;
            ss >> std::dec >> temp;
            *(double*)buffer = temp;
            break;
        }
        default:
            break;
    }
    if (ss.fail()) {
        throw MedException("Failed to convert string to memory: " + str);
    }
}

void hexStringToMemory(const std::string& str, ScanType type, Byte* buffer) {
    std::string sanitized = str;
    if (sanitized.find("0x") == 0 || sanitized.find("0X") == 0) {
        sanitized = sanitized.substr(2);
    }

    size_t length = scanTypeToSize(type) * 2;
    if (sanitized.length() > length) {
        sanitized = sanitized.substr(sanitized.length() - length);
    }

    std::stringstream ss(sanitized);
    ss << std::hex;

    switch (type) {
        case ScanType::Int8:
        case ScanType::UInt8: {
            int temp;
            ss >> temp;
            *(uint8_t*)buffer = (uint8_t)temp;
            break;
        }
        case ScanType::Int16:
        case ScanType::UInt16: {
            uint16_t temp;
            ss >> temp;
            *(uint16_t*)buffer = temp;
            break;
        }
        case ScanType::Int32:
        case ScanType::UInt32:
        case ScanType::Ptr32: {
            uint32_t temp;
            ss >> temp;
            *(uint32_t*)buffer = temp;
            break;
        }
        case ScanType::Int64:
        case ScanType::UInt64:
        case ScanType::Ptr64: {
            uint64_t temp;
            ss >> temp;
            *(uint64_t*)buffer = temp;
            break;
        }
        case ScanType::Float32: {
            // Hex to float is tricky with stringstream. 
            // We might need a different approach if we want literal hex to float.
            // But usually hex strings are for integers or pointers.
            uint32_t temp;
            ss >> temp;
            std::memcpy(buffer, &temp, sizeof(float));
            break;
        }
        case ScanType::Float64: {
            uint64_t temp;
            ss >> temp;
            std::memcpy(buffer, &temp, sizeof(double));
            break;
        }
        default:
            break;
    }
    if (ss.fail()) {
        throw MedException("Failed to convert hex string to memory: " + str);
    }
}

bool isHexString(const std::string& str) {
    return std::regex_match(str, std::regex("^0[xX][0-9a-fA-F]+"));
}

} // namespace MedUtil
