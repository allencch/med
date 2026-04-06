#include <regex>
#include <sstream>
#include <algorithm>
#include <cstring>
#include "med/ScanParser.hpp"
#include "med/MedCommon.hpp"
#include "med/MedException.hpp"
#include "med/Coder.hpp"

namespace StringUtil {
    std::string trim(const std::string& s) {
        if (s.empty()) return s;
        size_t first = s.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = s.find_last_not_of(" \t\n\r");
        return s.substr(first, (last - first + 1));
    }

    std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> tokens;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            std::string trimmed = trim(item);
            if (!trimmed.empty()) {
                tokens.push_back(trimmed);
            }
        }
        return tokens;
    }
}

namespace ScanParser {

const char* OP_REGEX = "^(>=|<=|<>|!=|!|=|>|<|\\?|~)";

OpType getOpType(const std::string& v) {
    std::string value = StringUtil::trim(v);
    std::regex r(OP_REGEX);
    std::smatch m;
    if (std::regex_search(value, m, r)) {
        std::string op = m.str();
        if (op == "?") return OpType::SnapshotSave;
        if (op == "<") return OpType::Lt;
        if (op == ">") return OpType::Gt;
        if (op == "=") return OpType::Eq;
        if (op == "!=" || op == "!") return OpType::Neq;
        if (op == "<=") return OpType::Le;
        if (op == ">=") return OpType::Ge;
        if (op == "<>") return OpType::Within;
        if (op == "~") return OpType::Around;
    }
    return OpType::Eq;
}

std::string getValue(const std::string& v) {
    std::string value = StringUtil::trim(v);
    std::regex r(OP_REGEX);
    return StringUtil::trim(std::regex_replace(value, r, ""));
}

std::vector<std::string> getValues(const std::string& v, char delimiter) {
    return StringUtil::split(getValue(v), delimiter);
}

Operands valueToOperands(const std::string& v, ScanType type, OpType op, EncodingType encoding) {
    std::vector<std::string> strValues;
    std::string valuePart = getValue(v);
    
    if (valuePart.empty()) {
        if (op == OpType::SnapshotSave || op == OpType::Eq || op == OpType::Neq || 
            op == OpType::Gt || op == OpType::Lt || op == OpType::Ge || op == OpType::Le) {
            return Operands();
        }
        throw MedException("Scan value is empty");
    }

    if (op == OpType::Within || op == OpType::Around) {
        strValues = StringUtil::split(valuePart, ' ');
    } else {
        strValues = StringUtil::split(valuePart, ',');
    }

    if (op == OpType::Around) {
        double center = std::stod(strValues[0]);
        double delta = (strValues.size() > 1) ? std::stod(strValues[1]) : 1.0;
        strValues = {std::to_string(center - delta), std::to_string(center + delta)};
    }

    std::vector<SizedBytes> data;
    size_t elementSize = MedUtil::scanTypeToSize(type);
    
    if (type == ScanType::String) {
        std::string encodedValue = valuePart;
        if (encoding == EncodingType::Big5) {
            encodedValue = convertFromUtf8(valuePart, "big5");
        }
        SizedBytes sb = SizedBytes::create(encodedValue.size() + 1);
        std::memcpy(sb.getBytes(), encodedValue.c_str(), encodedValue.size() + 1);
        data.push_back(sb);
    } else {
        for (const auto& s : strValues) {
            SizedBytes sb = SizedBytes::create(elementSize);
            MedUtil::stringToMemory(s, type, sb.getBytes());
            data.push_back(sb);
        }
    }

    return Operands(std::move(data));
}

} // namespace ScanParser
