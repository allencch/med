#include <iostream>
#include <unistd.h>
#include <algorithm>
#include "med/MemScanner.hpp"
#include "med/MemOperator.hpp"
#include "med/MedCommon.hpp"

MemScanner::MemScanner(pid_t pid) : pid_(pid), memio_(pid) {}

std::vector<ScanResult> MemScanner::scan(const ScanParams& params) {
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
    std::vector<ScanResult> newResults;
    std::mutex resultMutex;
    std::vector<std::future<void>> futures;

    const size_t CHUNK_SIZE = 1000;
    for (size_t i = 0; i < list.size(); i += CHUNK_SIZE) {
        size_t end = std::min(i + CHUNK_SIZE, list.size());
        futures.emplace_back(threadPool_.enqueue([this, &list, i, end, &params, &newResults, &resultMutex] {
            for (size_t j = i; j < end; ++j) {
                Address currentAddr = list[j].address;
                size_t typeSize = MedUtil::scanTypeToSize(params.type);

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

                try {
                    SizedBytes data = memio_.read(currentAddr, typeSize);
                    if (MemOperator::compare(data.getBytes(), params.type, params.operands, params.op)) {
                        std::lock_guard<std::mutex> lock(resultMutex);
                        newResults.push_back({currentAddr, params.type, data});
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

void MemScanner::setScope(Address start, Address end) {
    scope_ = {start, end};
}

void MemScanner::clearScope() {
    scope_ = {0, 0};
}

void MemScanner::scanMap(const AddressPair& map, const ScanParams& params, std::vector<ScanResult>& results, std::mutex& resultMutex) {
    size_t pageSize = getpagesize();
    size_t typeSize = MedUtil::scanTypeToSize(params.type);
    if (typeSize == 0) return;

    for (Address addr = map.first; addr < map.second; addr += pageSize) {
        size_t currentReadSize = std::min((size_t)pageSize, (size_t)(map.second - addr));
        try {
            // Read one page at a time to be efficient and thread-safe via MemIO
            SizedBytes page = memio_.read(addr, currentReadSize);
            const Byte* pageData = page.getBytes();

            for (size_t offset = 0; offset <= currentReadSize - typeSize; ++offset) {
                Address currentAddr = addr + offset;

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

                if (MemOperator::compare(pageData + offset, params.type, params.operands, params.op)) {
                    std::lock_guard<std::mutex> lock(resultMutex);
                    // For the result, we can either store the bytes or re-read
                    // Storing here is faster for small types
                    results.push_back({currentAddr, params.type, SizedBytes(pageData + offset, typeSize)});
                }
            }
        } catch (...) {
            // Ignore pages that fail to read (could be unmapped or permissions changed)
            continue;
        }
    }
}
