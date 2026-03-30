#include <string>
#include <cstdio>
#include <cxxtest/TestSuite.h>

#include "mem/Mem.hpp"

class TestMem : public CxxTest::TestSuite {
public:
  void testGetValue() {
    MemPtr mem = MemPtr(new Mem(4));
    mem->setValue(30);
    int value = mem->getValueAsInt();

    TS_ASSERT_EQUALS(value, 30);

    mem->setValue(40);
    value = mem->getValueAsInt();
    TS_ASSERT_EQUALS(value, 40);
  }
};
