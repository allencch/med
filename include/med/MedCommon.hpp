#ifndef MED_COMMON_HPP
#define MED_COMMON_HPP

#include <string>
#include <vector>
#include "med/MedTypes.hpp"

namespace MedUtil {
    ScanType stringToScanType(const std::string& scanType);
    std::string scanTypeToString(const ScanType& scanType);
    size_t scanTypeToSize(const ScanType& type);

    std::string intToHex(Address addr);
    Address hexToInt(const std::string& str);

    void stringToMemory(const std::string& str, ScanType type, Byte* buffer);
    void hexStringToMemory(const std::string& str, ScanType type, Byte* buffer);
    bool isHexString(const std::string& str);
}

#endif
