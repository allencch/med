#include <cxxtest/TestSuite.h>
#include <cstring>
#include <vector>
#include "med/MemOperator.hpp"
#include "med/ScanParser.hpp"

class TestArrayScan : public CxxTest::TestSuite {
public:
    void testCommaSeparatedArray() {
        // Value: 1, 2, 3 as Int32
        int32_t data[] = {1, 2, 3};
        
        std::string input = "1, 2, 3";
        Operands ops = ScanParser::valueToOperands(input, ScanType::Int32, ScanParser::OpType::Eq);
        
        // Confirm it parsed into 3 operands
        TS_ASSERT_EQUALS(ops.count(), 3);
        
        // Current implementation only compares the first element
        // So this will pass
        TS_ASSERT(MemOperator::compare(&data[0], ScanType::Int32, ops, ScanParser::OpType::Eq));
        
        // And this will also pass even though the sequence is wrong
        int32_t wrongData[] = {1, 0, 0};
        TS_ASSERT(MemOperator::compare(&wrongData[0], ScanType::Int32, ops, ScanParser::OpType::Eq));
        
        // If it were a proper array scan, the second one should fail.
    }

    void testStringComma() {
        std::string input = "hello, world";
        Operands ops = ScanParser::valueToOperands(input, ScanType::String, ScanParser::OpType::Eq);
        
        // Strings are NOT split by comma in ScanParser
        TS_ASSERT_EQUALS(ops.count(), 1);
        TS_ASSERT_EQUALS(std::string((char*)ops.getFirstOperand().getBytes()), "hello, world");
    }
};
