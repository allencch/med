#ifndef MED_MEM_SCANNER_HPP
#define MED_MEM_SCANNER_HPP

#include <vector>
#include <mutex>
#include <atomic>
#include "med/MedException.hpp"
#include "med/MedTypes.hpp"
#include "med/MemIO.hpp"
#include "med/Process.hpp"
#include "med/ThreadPool.hpp"
#include "med/Operands.hpp"
#include "med/ScanParser.hpp"
#include "med/ScanCommand.hpp"

struct ScanResult {
    Address address;
    ScanType type;
    SizedBytes data; // Comparison baseline
    SizedBytes liveData; // Current display value
};

struct ScanParams {
    Operands operands;
    ScanType type;
    ScanParser::OpType op;
    bool fastScan = false;
    std::vector<int> lastDigits;
    ScanCommand customScan;
};

struct SnapshotBlock {
    Address address;
    SizedBytes data;
};

class MemScanner {
public:
    explicit MemScanner(pid_t pid);
    
    std::vector<ScanResult> scan(const ScanParams& params);
    std::vector<ScanResult> filter(const std::vector<ScanResult>& list, const ScanParams& params);

    void setScope(Address start, Address end);
    void clearScope();

    void saveSnapshot();

private:
    void saveSnapshotInternal();
    class ScanLock {
    public:
        explicit ScanLock(std::atomic<bool>& scanning) : scanning_(scanning) {
            if (scanning_.exchange(true)) {
                throw MedException("A scan or filter operation is already in progress");
            }
        }
        ~ScanLock() {
            scanning_.store(false);
        }
    private:
        std::atomic<bool>& scanning_;
    };

    void scanMap(const AddressPair& map, const ScanParams& params, std::vector<ScanResult>& results, std::mutex& resultMutex);
    std::vector<ScanResult> filterSnapshot(const ScanParams& params);

    pid_t pid_;
    MemIO memio_;
    ThreadPool threadPool_;
    AddressPair scope_ = {0, 0};
    std::vector<SnapshotBlock> snapshotBlocks_;
    std::atomic<bool> isScanning_{false};
};

#endif
