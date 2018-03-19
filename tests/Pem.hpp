#include <string>
#include <cstdio>
#include <iostream>
#include <cxxtest/TestSuite.h>

#include "med/MedTypes.hpp"
#include "mem/Pem.hpp"

using namespace std;

class TestPem : public CxxTest::TestSuite {
public:
  void testGetValue() {
    MemIO* memio = new MemIO();

    int memory = 100;
    PemPtr pem = PemPtr(new Pem((Address)&memory, 4, memio));
    string value = pem->getValue("int32");
    TS_ASSERT_EQUALS(value, "100");

    delete memio;
  }
};
