#include <string>
#include <cstdio>
#include <iostream>
#include <cxxtest/TestSuite.h>

#include "mem/MemScanner.hpp"

using namespace std;

class TestMemScanner : public CxxTest::TestSuite {
public:
  void testScan() {
    MemScanner scanner;
    int memory[] = {100, 200, 100};

    auto buffer = ScanParser::valueToBytes("100", "int32");
    size_t size = std::get<1>(buffer);

    auto list = scanner.scanInner(std::get<0>(buffer), size, (Address)memory, 4 * 3, "int32", ScanParser::OpType::Eq);

    TS_ASSERT_EQUALS(list.size(), 2);

    delete[] std::get<0>(buffer);
  }
};
