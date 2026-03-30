#include <cxxtest/TestSuite.h>
#include <cstring>
#include <vector>
#include <unistd.h>

#include "med/MedTypes.hpp"
#include "med/SizedBytes.hpp"
#include "med/Maps.hpp"
#include "med/MedCommon.hpp"
#include "med/MemIO.hpp"
#include "med/Process.hpp"

class TestPhase1 : public CxxTest::TestSuite {
public:
    void testSizedBytes() {
        uint8_t raw[] = {0x01, 0x02, 0x03, 0x04};
        SizedBytes sb(raw, 4);
        TS_ASSERT_EQUALS(sb.getSize(), 4);
        TS_ASSERT(!sb.isEmpty());
        TS_ASSERT_EQUALS(sb.getBytes()[0], 0x01);
        TS_ASSERT_EQUALS(sb.getBytes()[3], 0x04);

        SizedBytes sb2 = SizedBytes::create(8);
        TS_ASSERT_EQUALS(sb2.getSize(), 8);
        std::memset(sb2.getBytes(), 0xFF, 8);
        TS_ASSERT_EQUALS(sb2.getBytes()[7], 0xFF);

        SizedBytes empty;
        TS_ASSERT(empty.isEmpty());
        TS_ASSERT_EQUALS(empty.getSize(), 0);
    }

    void testMaps() {
        Maps maps;
        maps.push({0x1000, 0x2000});
        maps.push({0x3000, 0x4000});

        TS_ASSERT_EQUALS(maps.size(), 2);
        TS_ASSERT_EQUALS(maps[0].first, 0x1000);
        TS_ASSERT(maps.hasPair({0x3000, 0x4000}));

        maps.trimByScope({0x1500, 0x3500});
        // 0x1000-0x2000 -> 0x1500-0x2000
        // 0x3000-0x4000 -> 0x3000-0x3500
        TS_ASSERT_EQUALS(maps.size(), 2);
        TS_ASSERT_EQUALS(maps[0].first, 0x1500);
        TS_ASSERT_EQUALS(maps[0].second, 0x2000);
        TS_ASSERT_EQUALS(maps[1].first, 0x3000);
        TS_ASSERT_EQUALS(maps[1].second, 0x3500);
    }

    void testMedUtil() {
        TS_ASSERT_EQUALS(MedUtil::stringToScanType("int32"), ScanType::Int32);
        TS_ASSERT_EQUALS(MedUtil::scanTypeToString(ScanType::Float64), "float64");
        TS_ASSERT_EQUALS(MedUtil::scanTypeToSize(ScanType::Int16), 2);

        TS_ASSERT_EQUALS(MedUtil::intToHex(0xABC), "0xabc");
        TS_ASSERT_EQUALS(MedUtil::hexToInt("0x123"), 0x123);
        TS_ASSERT_EQUALS(MedUtil::hexToInt("FF"), 0xFF);

        uint32_t buffer = 0;
        MedUtil::stringToMemory("1234", ScanType::Int32, (Byte*)&buffer);
        TS_ASSERT_EQUALS(buffer, 1234);

        MedUtil::hexStringToMemory("0x12345678", ScanType::Int32, (Byte*)&buffer);
        TS_ASSERT_EQUALS(buffer, 0x12345678);
    }

    void testMemIOLocal() {
        uint32_t target = 0xDEADBEEF;
        MemIO memIO(0); // Local

        SizedBytes data = memIO.read((Address)&target, 4);
        TS_ASSERT_EQUALS(data.getSize(), 4);
        TS_ASSERT_EQUALS(*(uint32_t*)data.getBytes(), 0xDEADBEEF);

        uint32_t newVal = 0xCAFEBABE;
        memIO.write((Address)&target, (Byte*)&newVal, 4);
        TS_ASSERT_EQUALS(target, 0xCAFEBABE);
    }

    void testProcessList() {
        auto list = Process::listAll();
        TS_ASSERT(!list.empty());
        // Find our own process
        pid_t myPid = getpid();
        bool found = false;
        for (const auto& p : list) {
            if (p.getPid() == myPid) {
                found = true;
                break;
            }
        }
        TS_ASSERT(found);
    }
};
