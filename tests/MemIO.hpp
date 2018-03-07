#include <string>
#include <cstdio>
#include <cxxtest/TestSuite.h>

#include "mem/MemIO.hpp"
#include "mem/Mem.hpp"

class TestMemIO : public CxxTest::TestSuite {
public:
  void testRead() {
    unsigned char ptr1[] = { 0x64, 0x65, 0x66 };
    MemIO memIO;
    MemPtr mem = memIO.read((Address)ptr1, 3);
    TS_ASSERT_EQUALS(mem->data[0], ptr1[0]);
    TS_ASSERT_EQUALS(mem->data[1], ptr1[1]);
    TS_ASSERT_EQUALS(mem->data[2], ptr1[2]);
    TS_ASSERT_EQUALS(mem->size, 3);
  }
};
