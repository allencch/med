#include <cxxtest/TestSuite.h>
#include <cstring>
#include <vector>
#include <unistd.h>

#include "med/MemScanner.hpp"
#include "med/ScanParser.hpp"
#include "med/MemOperator.hpp"

class TestPhase2 : public CxxTest::TestSuite {
public:
    void testScanParser() {
        std::string input = ">= 1234";
        TS_ASSERT_EQUALS(ScanParser::getOpType(input), ScanParser::OpType::Ge);
        TS_ASSERT_EQUALS(ScanParser::getValue(input), "1234");

        std::string input2 = "<> 100 200";
        TS_ASSERT_EQUALS(ScanParser::getOpType(input2), ScanParser::OpType::Within);
        Operands ops = ScanParser::valueToOperands(input2, ScanType::Int32, ScanParser::OpType::Within);
        TS_ASSERT_EQUALS(ops.count(), 2);
        TS_ASSERT_EQUALS(*(int32_t*)ops.getFirstOperand().getBytes(), 100);
        TS_ASSERT_EQUALS(*(int32_t*)ops.getSecondOperand().getBytes(), 200);
    }

    void testMemOperator() {
        int32_t val = 1000;
        Operands ops({SizedBytes((Byte*)&val, 4)});
        
        int32_t target = 1000;
        TS_ASSERT(MemOperator::compare(&target, ScanType::Int32, ops, ScanParser::OpType::Eq));
        
        target = 1001;
        TS_ASSERT(MemOperator::compare(&target, ScanType::Int32, ops, ScanParser::OpType::Gt));
        TS_ASSERT(!MemOperator::compare(&target, ScanType::Int32, ops, ScanParser::OpType::Eq));

        // Within
        int32_t low = 500, high = 1500;
        Operands opsWithin({SizedBytes((Byte*)&low, 4), SizedBytes((Byte*)&high, 4)});
        target = 1000;
        TS_ASSERT(MemOperator::compare(&target, ScanType::Int32, opsWithin, ScanParser::OpType::Within));
        target = 2000;
        TS_ASSERT(!MemOperator::compare(&target, ScanType::Int32, opsWithin, ScanParser::OpType::Within));
    }

    void testMemScannerLocal() {
        // We will scan our own memory for a specific value
        volatile int32_t targetValue = 0x1337BEEF;
        pid_t myPid = getpid();
        MemScanner scanner(myPid);

        ScanParams params;
        params.type = ScanType::Int32;
        params.op = ScanParser::OpType::Eq;
        int32_t val = 0x1337BEEF;
        params.operands = Operands({SizedBytes((Byte*)&val, 4)});
        
        auto results = scanner.scan(params);
        
        bool found = false;
        for (const auto& res : results) {
            if (res.address == (Address)&targetValue) {
                found = true;
                TS_ASSERT_EQUALS(*(int32_t*)res.data.getBytes(), 0x1337BEEF);
                break;
            }
        }
        TS_ASSERT(found);

        // Test filtering
        targetValue = 0xDEADBEEF;
        val = 0xDEADBEEF;
        params.operands = Operands({SizedBytes((Byte*)&val, 4)});
        auto filtered = scanner.filter(results, params);
        
        found = false;
        for (const auto& res : filtered) {
            if (res.address == (Address)&targetValue) {
                found = true;
                TS_ASSERT_EQUALS(*(int32_t*)res.data.getBytes(), 0xDEADBEEF);
                break;
            }
        }
        TS_ASSERT(found);
    }
};
