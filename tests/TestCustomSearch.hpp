#include <cxxtest/TestSuite.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include "med/ScanCommand.hpp"
#include "med/MedTypes.hpp"
#include "med/MemScanner.hpp"
#include "med/MemOperator.hpp"

class TestCustomSearch : public CxxTest::TestSuite {
public:
    void testSubCommandParsing() {
        SubCommand s1("i32:123");
        TS_ASSERT_EQUALS(s1.getCmd(), SubCommand::Command::Int32);
        TS_ASSERT_EQUALS(s1.getSize(), 4);

        SubCommand s2("s:'hello'");
        TS_ASSERT_EQUALS(s2.getCmd(), SubCommand::Command::Str);
        TS_ASSERT_EQUALS(s2.getSize(), 6); // Includes null terminator

        SubCommand s3("w:4");
        TS_ASSERT_EQUALS(s3.getCmd(), SubCommand::Command::Wildcard);
        TS_ASSERT_EQUALS(s3.getSize(), 4);

        SubCommand s4("1234", ScanType::Int32);
        TS_ASSERT_EQUALS(s4.getCmd(), SubCommand::Command::Noop);
        TS_ASSERT_EQUALS(s4.getSize(), 4);
    }

    void testSubCommandMatching() {
        uint32_t val = 1234;
        SubCommand s1("i32:1234");
        auto [match1, step1] = s1.match((Byte*)&val);
        TS_ASSERT(match1);
        TS_ASSERT_EQUALS(step1, 4);

        char str[] = "hello";
        SubCommand s2("s:'hello'");
        auto [match2, step2] = s2.match((Byte*)str);
        TS_ASSERT(match2);
        TS_ASSERT_EQUALS(step2, 6);

        SubCommand s3("w:2");
        auto [match3, step3] = s3.match((Byte*)str);
        TS_ASSERT(match3);
        TS_ASSERT_EQUALS(step3, 2);
    }

    void testScanCommand() {
        // Pattern: i32:1234, w:2, s:'abc'
        // Bytes: [D2 04 00 00] [XX XX] [61 62 63 00]
        uint8_t data[] = {0xD2, 0x04, 0x00, 0x00, 0xAA, 0xBB, 0x61, 0x62, 0x63, 0x00};
        
        ScanCommand sc("i32:1234, w:2, s:'abc'");
        TS_ASSERT_EQUALS(sc.getSize(), 10);
        TS_ASSERT(sc.match(data));

        uint8_t data2[] = {0xD2, 0x04, 0x00, 0x00, 0xAA, 0xBB, 0x61, 0x62, 0x64, 0x00};
        TS_ASSERT(!sc.match(data2));
    }

    void testScanCommandOperators() {
        uint8_t data[] = {10, 20};
        ScanCommand sc("i8:> 5, i8:< 25");
        TS_ASSERT(sc.match(data));

        ScanCommand sc2("i8:> 15, i8:< 25");
        TS_ASSERT(!sc2.match(data));
    }

    void testMemScannerCustom() {
        pid_t pid = getpid();
        MemScanner scanner(pid);

        // Allocate some memory to scan
        struct MyData {
            int32_t a;
            uint8_t b[2];
            char c[4];
        };
        MyData* data = new MyData();
        data->a = 0x12345678;
        data->b[0] = 0xAA;
        data->b[1] = 0xBB;
        std::strcpy(data->c, "abc");

        Address addr = (Address)data;
        scanner.setScope(addr, addr + sizeof(MyData));

        ScanParams params;
        params.type = ScanType::Custom;
        params.customScan = ScanCommand("i32:0x12345678, w:2, s:'abc'");
        
        auto results = scanner.scan(params);
        TS_ASSERT_EQUALS(results.size(), 1);
        TS_ASSERT_EQUALS(results[0].address, addr);

        delete data;
    }

    void testToStringCustom() {
        uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0xAA, 0xBB};
        std::string res = MemOperator::toString(data, 6, ScanType::Custom);
        TS_ASSERT_EQUALS(res, "12 34 56 78 AA BB ");
    }
};
