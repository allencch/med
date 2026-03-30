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
    TS_ASSERT_EQUALS(mem->getData()[0], ptr1[0]);
    TS_ASSERT_EQUALS(mem->getData()[1], ptr1[1]);
    TS_ASSERT_EQUALS(mem->getData()[2], ptr1[2]);
    TS_ASSERT_EQUALS(mem->getSize(), 3);
  }

  void testWrite() {
    unsigned char ptr1[] = { 0x64, 0x65, 0x66 };
    MemIO memIO;
    MemPtr mem = memIO.read((Address)ptr1, 3);
    mem->setValue(0x6867);
    memIO.write((Address)ptr1, mem, 2);

    TS_ASSERT_EQUALS(ptr1[0], 0x67);
    TS_ASSERT_EQUALS(ptr1[1], 0x68);
    TS_ASSERT_EQUALS(ptr1[2], 0x66);
  }
};
