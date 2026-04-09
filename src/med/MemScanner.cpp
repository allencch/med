#include <iostream>
#include <unistd.h>
#include <algorithm>
#include "med/MemScanner.hpp"
#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"

MemScanner::MemScanner(pid_t pid) : pid_(pid), memio_(pid) {}

std::vector<ScanResult> MemScanner::scan(const ScanParams& params) {
    ScanLock lock(isScanning_);
    if (params.op == ScanParser::OpType::SnapshotSave) {
        saveSnapshotInternal();
        return {};
    }

    Process proc(pid_, "");
    Maps maps = proc.getMaps();
    if (scope_.first && scope_.second) {
        maps.trimByScope(scope_);
    }

    std::vector<ScanResult> results;
    std::mutex resultMutex;
    std::vector<std::future<void>> futures;

    for (size_t i = 0; i < maps.size(); ++i) {
        futures.emplace_back(threadPool_.enqueue([this, map = maps[i], &params, &results, &resultMutex] {
            scanMap(map, params, results, resultMutex);
        }));
    }

    for (auto& f : futures) {
        f.get();
    }

    std::sort(results.begin(), results.end(), [](const ScanResult& a, const ScanResult& b) {
        return a.address < b.address;
    });

    return results;
}

std::vector<ScanResult> MemScanner::filter(const std::vector<ScanResult>& list, const ScanParams& params) {
    ScanLock lock(isScanning_);
    if (params.op == ScanParser::OpType::SnapshotSave) {
        saveSnapshotInternal();
        // If we have a list, we might want to refresh it as well
        if (list.empty()) return {};
        
        std::vector<ScanResult> refreshed = list;
        MemIO memio(pid_);
        for (auto& res : refreshed) {
            try {
                res.data = memio.read(res.address, res.data.getSize());
            } catch (...) {}
        }
        return refreshed;
    }

    if (list.empty() && !snapshotBlocks_.empty()) {
        return filterSnapshot(params);
    }

    std::vector<ScanResult> newResults;
    std::mutex resultMutex;
    std::vector<std::future<void>> futures;

    const size_t CHUNK_SIZE = 1000;
    for (size_t i = 0; i < list.size(); i += CHUNK_SIZE) {
        size_t end = std::min(i + CHUNK_SIZE, list.size());
        futures.emplace_back(threadPool_.enqueue([this, &list, i, end, &params, &newResults, &resultMutex] {
            size_t typeSize = (params.type == ScanType::Custom) ? params.customScan.getFirstSize() : MedUtil::scanTypeToSize(params.type);
            if (typeSize == 0) typeSize = 1;
            size_t totalSize = (params.type == ScanType::Custom) ? params.customScan.getSize() : 
                               (params.operands.count() > 0 ? params.operands.getTotalSize() : typeSize);
            if (totalSize == 0) return;

            std::vector<ScanResult> localResults;

            for (size_t j = i; j < end; ++j) {
                Address currentAddr = list[j].address;

                // Fast scan check
                if (params.fastScan && (currentAddr % typeSize != 0)) continue;

                // Last digits check
                if (!params.lastDigits.empty()) {
                    bool matchDigit = false;
                    for (int digit : params.lastDigits) {
                        if (currentAddr % 16 == (Address)digit) {
                            matchDigit = true;
                            break;
                        }
                    }
                    if (!matchDigit) continue;
                }

                try {
                    SizedBytes data = memio_.read(currentAddr, totalSize);
                    bool match = false;
                    if (params.type == ScanType::Custom) {
                        match = params.customScan.match(data.getBytes());
                    } else if (params.operands.count() > 0) {
                        match = MemOperator::compare(data.getBytes(), params.type, params.operands, params.op);
                    } else {
                        match = MemOperator::compare(data.getBytes(), list[j].data.getBytes(), params.type, params.op);
                    }

                    if (match) {
                        localResults.push_back({currentAddr, params.type, data, data});
                    }
                } catch (...) {}
            }

            if (!localResults.empty()) {
                std::lock_guard<std::mutex> lock(resultMutex);
                newResults.insert(newResults.end(), localResults.begin(), localResults.end());
            }
        }));
    }

    for (auto& f : futures) {
        f.get();
    }

    std::sort(newResults.begin(), newResults.end(), [](const ScanResult& a, const ScanResult& b) {
        return a.address < b.address;
    });

    return newResults;
}

void MemScanner::saveSnapshot() {
    ScanLock lock(isScanning_);
    saveSnapshotInternal();
}

void MemScanner::saveSnapshotInternal() {
    snapshotBlocks_.clear();
    Process proc(pid_, "");
    Maps maps = proc.getMaps();
    if (scope_.first && scope_.second) {
        maps.trimByScope(scope_);
    }

    std::mutex snapshotMutex;
    std::vector<std::future<void>> futures;
    const size_t CHUNK_SIZE = 1024 * 1024; // 1MB chunks for snapshot

    for (size_t i = 0; i < maps.size(); ++i) {
        futures.emplace_back(threadPool_.enqueue([this, map = maps[i], &snapshotMutex, CHUNK_SIZE] {
            std::vector<SnapshotBlock> localBlocks;
            for (Address addr = map.first; addr < map.second; addr += CHUNK_SIZE) {
                size_t currentReadSize = std::min(CHUNK_SIZE, (size_t)(map.second - addr));
                try {
                    SizedBytes data = memio_.read(addr, currentReadSize);
                    localBlocks.push_back({addr, data});
                } catch (...) {}
            }
            if (!localBlocks.empty()) {
                std::lock_guard<std::mutex> lock(snapshotMutex);
                snapshotBlocks_.insert(snapshotBlocks_.end(), localBlocks.begin(), localBlocks.end());
            }
        }));
    }

    for (auto& f : futures) {
        f.get();
    }
}

std::vector<ScanResult> MemScanner::filterSnapshot(const ScanParams& params) {
    std::vector<ScanResult> results;
    std::mutex resultMutex;
    std::vector<std::future<void>> futures;

    size_t typeSize = MedUtil::scanTypeToSize(params.type);

    for (size_t i = 0; i < snapshotBlocks_.size(); ++i) {
        futures.emplace_back(threadPool_.enqueue([this, i, &params, &results, &resultMutex, typeSize] {
            const auto& block = snapshotBlocks_[i];
            try {
                SizedBytes currentData = memio_.read(block.address, block.data.getSize());
                const Byte* oldPtr = block.data.getBytes();
                const Byte* newPtr = currentData.getBytes();

                std::vector<ScanResult> localResults;

                for (size_t offset = 0; offset <= block.data.getSize() - typeSize; ++offset) {
                    Address currentAddr = block.address + offset;

                    // Fast scan check
                    if (params.fastScan && (currentAddr % typeSize != 0)) continue;

                    // Last digits check
                    if (!params.lastDigits.empty()) {
                        bool match = false;
                        for (int digit : params.lastDigits) {
                            if (currentAddr % 16 == (Address)digit) {
                                match = true;
                                break;
                            }
                        }
                        if (!match) continue;
                    }

                    if (MemOperator::compare(newPtr + offset, oldPtr + offset, params.type, params.op)) {
                        SizedBytes newData(newPtr + offset, typeSize);
                        localResults.push_back({currentAddr, params.type, newData, newData});
                    }
                }

                if (!localResults.empty()) {
                    std::lock_guard<std::mutex> lock(resultMutex);
                    results.insert(results.end(), localResults.begin(), localResults.end());
                }
            } catch (...) {}
        }));
    }

    for (auto& f : futures) {
        f.get();
    }

    std::sort(results.begin(), results.end(), [](const ScanResult& a, const ScanResult& b) {
        return a.address < b.address;
    });

    return results;
}

void MemScanner::setScope(Address start, Address end) {
    scope_ = {start, end};
}

void MemScanner::clearScope() {
    scope_ = {0, 0};
}

void MemScanner::scanMap(const AddressPair& map, const ScanParams& params, std::vector<ScanResult>& results, std::mutex& resultMutex) {
    const size_t CHUNK_SIZE = 256 * 1024; // 256KB chunks for better throughput
    size_t typeSize = (params.type == ScanType::Custom) ? params.customScan.getFirstSize() : MedUtil::scanTypeToSize(params.type);
    if (typeSize == 0) typeSize = 1;
    size_t totalSize = (params.type == ScanType::Custom) ? params.customScan.getSize() : params.operands.getTotalSize();
    if (totalSize == 0) return;

    for (Address addr = map.first; addr < map.second; addr += CHUNK_SIZE) {
        size_t currentChunkSize = std::min(CHUNK_SIZE, (size_t)(map.second - addr));
        // To handle cross-boundary matches, we need to read slightly more than CHUNK_SIZE
        // if there's more data in the map.
        size_t readSize = currentChunkSize;
        if (addr + readSize + totalSize - 1 <= map.second) {
            readSize += totalSize - 1;
        }

        try {
            SizedBytes page = memio_.read(addr, readSize);
            const Byte* pageData = page.getBytes();
            size_t limit = page.getSize() >= totalSize ? page.getSize() - totalSize : 0;

            std::vector<ScanResult> localResults;

            for (size_t offset = 0; offset <= limit; ++offset) {
                Address currentAddr = addr + offset;

                // Fast scan check
                if (params.fastScan && (currentAddr % typeSize != 0)) continue;

                // Last digits check
                if (!params.lastDigits.empty()) {
                    bool matchDigit = false;
                    for (int digit : params.lastDigits) {
                        if (currentAddr % 16 == (Address)digit) {
                            matchDigit = true;
                            break;
                        }
                    }
                    if (!matchDigit) continue;
                }

                bool match = false;
                if (params.type == ScanType::Custom) {
                    match = params.customScan.match(pageData + offset);
                } else {
                    match = MemOperator::compare(pageData + offset, params.type, params.operands, params.op);
                }

                if (match) {
                    SizedBytes data(pageData + offset, totalSize);
                    localResults.push_back({currentAddr, params.type, data, data});
                }
            }

            if (!localResults.empty()) {
                std::lock_guard<std::mutex> lock(resultMutex);
                results.insert(results.end(), localResults.begin(), localResults.end());
            }
        } catch (...) {
            continue;
        }
    }
}
