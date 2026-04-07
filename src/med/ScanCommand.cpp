#include <regex>
#include <algorithm>
#include <cstring>
#include "med/ScanCommand.hpp"
#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"

namespace {
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

    std::string extractString(const std::string& s) {
        std::string value = trim(s);
        std::regex r(SubCommand::CMD_STRING);
        std::smatch match;
        if (std::regex_search(value, match, r)) {
            return match[1];
        }
        return "";
    }

    int extractNumber(const std::string& s) {
        std::string value = trim(s);
        std::regex r(":\\s*?(\\d+)");
        std::smatch match;
        if (std::regex_search(value, match, r)) {
            std::string matched = match[1];
            if (!matched.empty()) {
                return std::stoi(matched);
            }
        }
        return 0;
    }
}

SubCommand::SubCommand(const std::string& s, ScanType fallbackType, EncodingType encoding) {
    cmd_ = parseCmd(s);
    wildcardSteps_ = 0;
    std::string stripped = stripCommand(s);
    op_ = ScanParser::getOpType(stripped);
    std::string valueOnly = ScanParser::getValue(stripped);

    switch (cmd_) {
        case Command::Int8:
            type_ = ScanType::Int8;
            operands_ = ScanParser::valueToOperands(stripped, type_, op_, encoding);
            break;
        case Command::Int16:
            type_ = ScanType::Int16;
            operands_ = ScanParser::valueToOperands(stripped, type_, op_, encoding);
            break;
        case Command::Int32:
            type_ = ScanType::Int32;
            operands_ = ScanParser::valueToOperands(stripped, type_, op_, encoding);
            break;
        case Command::Int64:
            type_ = ScanType::Int64;
            operands_ = ScanParser::valueToOperands(stripped, type_, op_, encoding);
            break;
        case Command::Float32:
            type_ = ScanType::Float32;
            operands_ = ScanParser::valueToOperands(stripped, type_, op_, encoding);
            break;
        case Command::Float64:
            type_ = ScanType::Float64;
            operands_ = ScanParser::valueToOperands(stripped, type_, op_, encoding);
            break;
        case Command::Str: {
            type_ = ScanType::String;
            std::string valueStr = extractString(s);
            operands_ = ScanParser::valueToOperands(valueStr, type_, op_, encoding);
            break;
        }
        case Command::Wildcard:
            type_ = ScanType::Int8;
            wildcardSteps_ = extractNumber(s);
            break;
        case Command::Noop:
            type_ = fallbackType;
            operands_ = ScanParser::valueToOperands(stripped, type_, op_, encoding);
            break;
    }
}

SubCommand::Command SubCommand::parseCmd(const std::string& s) {
    auto cmd = getCmdString(s);
    if (cmd == "s:") return Command::Str;
    if (cmd == "i8:") return Command::Int8;
    if (cmd == "i16:") return Command::Int16;
    if (cmd == "i32:") return Command::Int32;
    if (cmd == "i64:") return Command::Int64;
    if (cmd == "f32:") return Command::Float32;
    if (cmd == "f64:") return Command::Float64;
    if (cmd == "w:") return Command::Wildcard;
    return Command::Noop;
}

std::string SubCommand::getCmdString(const std::string& s) {
    std::string value = trim(s);
    std::regex r(SubCommand::CMD_REGEX);
    std::smatch m;
    if (std::regex_search(value, m, r)) {
        return m.str();
    }
    return "";
}

std::string SubCommand::stripCommand(const std::string& s) const {
    std::string cmdStr = getCmdString(s);
    if (cmdStr.empty()) return s;
    std::string copy = s;
    size_t pos = copy.find(cmdStr);
    if (pos != std::string::npos) {
        copy.erase(pos, cmdStr.length());
    }
    return trim(copy);
}

std::tuple<bool, size_t> SubCommand::match(const Byte* address) const {
    size_t size = getSize();
    if (cmd_ == Command::Wildcard) {
        return std::make_tuple(true, size);
    }
    
    ScanType type;
    switch (cmd_) {
        case Command::Int8: type = ScanType::Int8; break;
        case Command::Int16: type = ScanType::Int16; break;
        case Command::Int32: type = ScanType::Int32; break;
        case Command::Int64: type = ScanType::Int64; break;
        case Command::Float32: type = ScanType::Float32; break;
        case Command::Float64: type = ScanType::Float64; break;
        case Command::Str: type = ScanType::String; break;
        default: type = ScanType::Int32; break; // fallback
    }

    bool matchResult = MemOperator::compare(address, type, operands_, op_);
    return std::make_tuple(matchResult, size);
}

size_t SubCommand::getSize() const {
    if (cmd_ == Command::Wildcard) return wildcardSteps_;
    return operands_.getTotalSize();
}

ScanType SubCommand::getScanType() const {
    return type_;
}

SubCommand::Command SubCommand::getCmd() const {
    return cmd_;
}

ScanCommand::ScanCommand(const std::string& s, ScanType fallbackType, EncodingType encoding) {
    auto values = split(s, ',');
    for (const auto& value : values) {
        subCommands_.emplace_back(value, fallbackType, encoding);
    }
    size_ = 0;
    for (const auto& sc : subCommands_) {
        size_ += sc.getSize();
    }
}

bool ScanCommand::match(const Byte* address) const {
    const Byte* ptr = address;
    for (const auto& sc : subCommands_) {
        auto [match, step] = sc.match(ptr);
        if (!match) return false;
        ptr += step;
    }
    return true;
}

size_t ScanCommand::getSize() const {
    return size_;
}

size_t ScanCommand::getFirstSize() const {
    if (subCommands_.empty()) return 1;
    size_t s = subCommands_[0].getSize();
    return s > 0 ? s : 1;
}

ScanType ScanCommand::getFirstScanType() const {
    if (subCommands_.empty()) return ScanType::Int32;
    return subCommands_[0].getScanType();
}

const std::vector<SubCommand>& ScanCommand::getSubCommands() const {
    return subCommands_;
}
