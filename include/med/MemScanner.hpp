#ifndef MED_MEM_SCANNER_HPP
#define MED_MEM_SCANNER_HPP

#include <vector>
#include <mutex>
#include "med/MedTypes.hpp"
#include "med/MemIO.hpp"
#include "med/Process.hpp"
#include "med/ThreadPool.hpp"
#include "med/Operands.hpp"
#include "med/ScanParser.hpp"

struct ScanResult {
    Address address;
    ScanType type;
    SizedBytes data;
};

struct ScanParams {
    Operands operands;
    ScanType type;
    ScanParser::OpType op;
    bool fastScan = false;
    std::vector<int> lastDigits;
};

class MemScanner {
public:
    explicit MemScanner(pid_t pid);
    
    std::vector<ScanResult> scan(const ScanParams& params);
    std::vector<ScanResult> filter(const std::vector<ScanResult>& list, const ScanParams& params);

    void setScope(Address start, Address end);
    void clearScope();

private:
    void scanMap(const AddressPair& map, const ScanParams& params, std::vector<ScanResult>& results, std::mutex& resultMutex);

    pid_t pid_;
    MemIO memio_;
    ThreadPool threadPool_;
    AddressPair scope_ = {0, 0};
};

#endif
