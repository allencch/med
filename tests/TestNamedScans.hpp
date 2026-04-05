#include <cxxtest/TestSuite.h>
#include "med/NamedScans.hpp"

using namespace med;

class TestNamedScans : public CxxTest::TestSuite {
public:
    void testBasicNamedScans() {
        NamedScans ns;
        TS_ASSERT_EQUALS(ns.getActiveName(), NamedScans::DEFAULT_NAME);
        TS_ASSERT_EQUALS(ns.getNames().size(), 1);
        TS_ASSERT_EQUALS(ns.getNames()[0], NamedScans::DEFAULT_NAME);
        
        ns.addNewScan("Scan1", ScanType::Float32);
        TS_ASSERT_EQUALS(ns.getNames().size(), 2);
        
        ns.setActiveName("Scan1");
        TS_ASSERT_EQUALS(ns.getActiveName(), "Scan1");
        TS_ASSERT_EQUALS(ns.getActiveType(), ScanType::Float32);
        
        std::vector<ScanResult> results = {
            { 0x1000, ScanType::Float32, SizedBytes::create(4) },
            { 0x2000, ScanType::Float32, SizedBytes::create(4) }
        };
        ns.setActiveResults(results);
        TS_ASSERT_EQUALS(ns.getActiveResults().size(), 2);
        TS_ASSERT_EQUALS(ns.getActiveResults()[0].address, 0x1000);
        
        ns.setActiveName(NamedScans::DEFAULT_NAME);
        TS_ASSERT_EQUALS(ns.getActiveResults().size(), 0);
        TS_ASSERT_EQUALS(ns.getActiveType(), ScanType::UInt32);
        
        ns.remove("Scan1");
        TS_ASSERT_EQUALS(ns.getNames().size(), 1);
        TS_ASSERT_EQUALS(ns.getActiveName(), NamedScans::DEFAULT_NAME);
    }
};
