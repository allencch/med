#ifndef MED_TYPES_HPP
#define MED_TYPES_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

typedef uint8_t Byte;
typedef uintptr_t Address;

enum class ScanType {
    Int8,
    Int16,
    Int32,
    Int64,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Float32,
    Float64,
    String,
    Custom,
    Ptr32,
    Ptr64,
    Unknown
};

const int MAX_STRING_SIZE = 1079;

namespace ScanTypeString {
    const std::string Int8 = "int8";
    const std::string Int16 = "int16";
    const std::string Int32 = "int32";
    const std::string Int64 = "int64";
    const std::string UInt8 = "uint8";
    const std::string UInt16 = "uint16";
    const std::string UInt32 = "uint32";
    const std::string UInt64 = "uint64";
    const std::string Float32 = "float32";
    const std::string Float64 = "float64";
    const std::string String = "string";
    const std::string Custom = "custom";
    const std::string Ptr32 = "ptr32";
    const std::string Ptr64 = "ptr64";
    const std::string Unknown = "unknown";
}

enum class EncodingType {
    Default,
    Big5
};

typedef std::pair<Address, Address> AddressPair;
typedef std::vector<AddressPair> AddressPairs;

typedef std::shared_ptr<Byte[]> BytePtr;

namespace ScanParser {
    enum class OpType {
        Eq,
        Gt,
        Lt,
        Neq,
        Ge,
        Le,
        Within,
        Around,
        SnapshotSave
    };
}

typedef std::vector<int> Integers;

#endif
