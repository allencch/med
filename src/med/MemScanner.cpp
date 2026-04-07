#include <iostream>
#include <unistd.h>
#include <algorithm>
#include "med/MemScanner.hpp"
#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"

MemScanner::MemScanner(pid_t pid) : pid_(pid), memio_(pid) {}

std::vector<ScanResult> MemScanner::scan(const ScanParams& params) {
    if (params.op == ScanParser::OpType::SnapshotSave) {
        saveSnapshot();
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
    if (params.op == ScanParser::OpType::SnapshotSave) {
        saveSnapshot();
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
                        std::lock_guard<std::mutex> lock(resultMutex);
                        newResults.push_back({currentAddr, params.type, data, data});
                    }
                } catch (...) {}
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
    snapshotBlocks_.clear();
    Process proc(pid_, "");
    Maps maps = proc.getMaps();
    if (scope_.first && scope_.second) {
        maps.trimByScope(scope_);
    }

    std::mutex snapshotMutex;
    std::vector<std::future<void>> futures;

    for (size_t i = 0; i < maps.size(); ++i) {
        futures.emplace_back(threadPool_.enqueue([this, map = maps[i], &snapshotMutex] {
            size_t pageSize = getpagesize();
            for (Address addr = map.first; addr < map.second; addr += pageSize) {
                size_t currentReadSize = std::min((size_t)pageSize, (size_t)(map.second - addr));
                try {
                    SizedBytes data = memio_.read(addr, currentReadSize);
                    std::lock_guard<std::mutex> lock(snapshotMutex);
                    snapshotBlocks_.push_back({addr, data});
                } catch (...) {}
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
                        std::lock_guard<std::mutex> lock(resultMutex);
                        SizedBytes newData(newPtr + offset, typeSize);
                        results.push_back({currentAddr, params.type, newData, newData});
                    }
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
    size_t pageSize = getpagesize();
    size_t typeSize = (params.type == ScanType::Custom) ? params.customScan.getFirstSize() : MedUtil::scanTypeToSize(params.type);
    if (typeSize == 0) typeSize = 1;
    size_t totalSize = (params.type == ScanType::Custom) ? params.customScan.getSize() : params.operands.getTotalSize();
    if (totalSize == 0) return;

    for (Address addr = map.first; addr < map.second; addr += pageSize) {
        size_t currentReadSize = std::min((size_t)pageSize, (size_t)(map.second - addr));
        try {
            // Read one page at a time to be efficient and thread-safe via MemIO
            SizedBytes page = memio_.read(addr, currentReadSize);
            const Byte* pageData = page.getBytes();

            for (size_t offset = 0; offset <= currentReadSize - totalSize; ++offset) {
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
                    std::lock_guard<std::mutex> lock(resultMutex);
                    SizedBytes data(pageData + offset, totalSize);
                    results.push_back({currentAddr, params.type, data, data});
                }
            }
        } catch (...) {
            // Ignore pages that fail to read (could be unmapped or permissions changed)
            continue;
        }
    }
}
