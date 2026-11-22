#include <cxxtest/TestSuite.h>
#include <unistd.h>
#include <sys/mman.h>
#include <vector>
#include <algorithm>
#include "med/MemScanner.hpp"
#include "med/ScanParser.hpp"

class TestLastDigitScope : public CxxTest::TestSuite {
public:
    void testIncludeWritableFileBackedMemory() {
        int fd = memfd_create("med-scan-test", MFD_CLOEXEC);
        TS_ASSERT(fd >= 0);
        if (fd < 0) return;

        int32_t target = 0x12345678;
        ssize_t written = write(fd, &target, sizeof(target));
        void* mapping = mmap(nullptr, sizeof(target), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
        TS_ASSERT_EQUALS(written, sizeof(target));
        TS_ASSERT_DIFFERS(mapping, MAP_FAILED);
        if (mapping == MAP_FAILED) return;

        Address start = reinterpret_cast<Address>(mapping);
        MemScanner scanner(getpid());
        ScanParams params;
        params.type = ScanType::Int32;
        params.op = ScanParser::OpType::Eq;
        params.operands = Operands({SizedBytes(reinterpret_cast<Byte*>(&target), sizeof(target))});

        scanner.setScope(start, start + sizeof(target));
        TS_ASSERT(scanner.scan(params).empty());

        scanner.setIncludeFileBackedInScope(true);
        auto results = scanner.scan(params);
        TS_ASSERT_EQUALS(results.size(), 1);
        if (!results.empty()) TS_ASSERT_EQUALS(results.front().address, start);

        scanner.clearScope();
        params.fastScan = true;
        results = scanner.scan(params);
        TS_ASSERT(std::any_of(results.begin(), results.end(), [start](const ScanResult& result) {
            return result.address == start;
        }));

        scanner.setScope(0, start);
        results = scanner.scan(params);
        TS_ASSERT(std::none_of(results.begin(), results.end(), [start](const ScanResult& result) {
            return result.address >= start;
        }));

        scanner.setScope(start, start + sizeof(target));
        int protectResult = mprotect(mapping, sizeof(target), PROT_READ);
        TS_ASSERT_EQUALS(protectResult, 0);
        TS_ASSERT(scanner.scan(params).empty());
        munmap(mapping, sizeof(target));
    }

    void testLastDigitScope() {
        volatile int32_t val1 = 0x12345678;
        volatile int32_t val2 = 0x12345678;
        
        pid_t pid = getpid();
        MemScanner scanner(pid);
        
        Address addr1 = (Address)&val1;
        Address addr2 = (Address)&val2;
        
        Address start = std::min(addr1, addr2);
        Address end = std::max(addr1, addr2) + 4;
        
        scanner.setScope(start, end);
        
        ScanParams params;
        params.type = ScanType::Int32;
        params.op = ScanParser::OpType::Eq;
        int32_t target = 0x12345678;
        params.operands = Operands({SizedBytes((Byte*)&target, 4)});
        
        int digit1 = addr1 % 16;
        int digit2 = addr2 % 16;
        
        params.lastDigits = {digit1, digit2};
        
        auto results = scanner.scan(params);
        
        bool found1 = false;
        bool found2 = false;
        for (const auto& res : results) {
            if (res.address == addr1) found1 = true;
            if (res.address == addr2) found2 = true;
        }
        
        TS_ASSERT(found1);
        TS_ASSERT(found2);
        TS_ASSERT(results.size() >= 2);

        // Test filtering out
        params.lastDigits = {(digit1 + 1) % 16, (digit2 + 1) % 16};
        // Ensure we don't accidentally match by +1
        if (params.lastDigits[0] == digit1) params.lastDigits[0] = (digit1 + 2) % 16;
        if (params.lastDigits[1] == digit2) params.lastDigits[1] = (digit2 + 2) % 16;
        
        auto results2 = scanner.scan(params);
        for (const auto& res : results2) {
            TS_ASSERT_DIFFERS(res.address, addr1);
            TS_ASSERT_DIFFERS(res.address, addr2);
        }
    }
};
