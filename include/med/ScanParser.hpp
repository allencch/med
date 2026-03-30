#ifndef MED_SCAN_PARSER_HPP
#define MED_SCAN_PARSER_HPP

#include <string>
#include <vector>
#include "med/MedTypes.hpp"
#include "med/SizedBytes.hpp"
#include "med/Operands.hpp"

namespace ScanParser {
    OpType getOpType(const std::string& v);
    std::string getValue(const std::string& v);
    std::vector<std::string> getValues(const std::string& v, char delimiter = ',');
    
    Operands valueToOperands(const std::string& v, ScanType type, OpType op);
}

namespace StringUtil {
    std::string trim(const std::string& s);
    std::vector<std::string> split(const std::string& s, char delim);
}

#endif
