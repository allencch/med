#ifndef MED_SCAN_COMMAND_HPP
#define MED_SCAN_COMMAND_HPP

#include <string>
#include <vector>
#include <tuple>
#include "med/MedTypes.hpp"
#include "med/Operands.hpp"
#include "med/ScanParser.hpp"

class SubCommand {
public:
    enum class Command {
        Noop,
        Str,
        Int8,
        Int16,
        Int32,
        Int64,
        Float32,
        Float64,
        Wildcard
    };

    static constexpr const char* CMD_REGEX = "^(s|w|i8|i16|i32|i64|f32|f64):";
    static constexpr const char* CMD_STRING = "\\s*?'(.+?)'";

    SubCommand(const std::string& s, ScanType fallbackType = ScanType::Int32, EncodingType encoding = EncodingType::Default);

    std::tuple<bool, size_t> match(const Byte* address) const;
    size_t getSize() const;
    Command getCmd() const;
    ScanType getScanType() const;

private:
    Command cmd_;
    ScanType type_;
    Operands operands_;
    ScanParser::OpType op_;
    size_t wildcardSteps_ = 0;

    static Command parseCmd(const std::string& s);
    static std::string getCmdString(const std::string& s);
    std::string stripCommand(const std::string& s) const;
};

class ScanCommand {
public:
    ScanCommand() = default;
    ScanCommand(const std::string& s, ScanType fallbackType = ScanType::Int32, EncodingType encoding = EncodingType::Default);

    bool match(const Byte* address) const;
    size_t getSize() const;
    size_t getFirstSize() const;
    ScanType getFirstScanType() const;
    const std::vector<SubCommand>& getSubCommands() const;

private:
    std::vector<SubCommand> subCommands_;
    size_t size_ = 0;
};

#endif
