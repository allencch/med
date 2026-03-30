#include <iostream>
#include "mem/Maps.hpp"

class Testmaps : public CxxTest::TestSuite {
public:
  void testTrimByScope_onePair() {
    Maps maps;
    AddressPair pair1(10, 20);
    maps.push(pair1);

    AddressPair scope(11, 21);
    maps.trimByScope(scope);
    TS_ASSERT_EQUALS(maps.size(), 1);
    TS_ASSERT_EQUALS(maps[0].first, 11);
    TS_ASSERT_EQUALS(maps[0].second, 20);

    maps.clear();
    maps.push(pair1);
    scope = make_pair(11, 19);
    maps.trimByScope(scope);
    TS_ASSERT_EQUALS(maps.size(), 1);
    TS_ASSERT_EQUALS(maps[0].first, 11);
    TS_ASSERT_EQUALS(maps[0].second, 19);

    maps.clear();
    maps.push(pair1);
    scope = make_pair(9, 19);
    maps.trimByScope(scope);
    TS_ASSERT_EQUALS(maps.size(), 1);
    TS_ASSERT_EQUALS(maps[0].first, 10);
    TS_ASSERT_EQUALS(maps[0].second, 19);

    maps.clear();
    maps.push(pair1);
    scope = make_pair(9, 21);
    maps.trimByScope(scope);
    TS_ASSERT_EQUALS(maps.size(), 1);
    TS_ASSERT_EQUALS(maps[0].first, 10);
    TS_ASSERT_EQUALS(maps[0].second, 20);
  }

  void testTrimByScope_twoPairs() {
    Maps maps;
    AddressPair pair1(10, 20);
    AddressPair pair2(30, 40);
    maps.push(pair1);
    maps.push(pair2);

    AddressPair scope;
    scope = make_pair(9, 19);
    maps.trimByScope(scope);
    TS_ASSERT_EQUALS(maps.size(), 1);
    TS_ASSERT_EQUALS(maps[0].first, 10);
    TS_ASSERT_EQUALS(maps[0].second, 19);

    maps.clear();
    maps.push(pair1);
    maps.push(pair2);
    scope = make_pair(20, 30);
    maps.trimByScope(scope);
    TS_ASSERT_EQUALS(maps.size(), 2);
    TS_ASSERT_EQUALS(maps[0].first, 20);
    TS_ASSERT_EQUALS(maps[0].second, 20);
    TS_ASSERT_EQUALS(maps[1].first, 30);
    TS_ASSERT_EQUALS(maps[1].second, 30);

    maps.clear();
    maps.push(pair1);
    maps.push(pair2);
    scope = make_pair(21, 29);
    maps.trimByScope(scope);
    TS_ASSERT_EQUALS(maps.size(), 0);

    maps.clear();
    maps.push(pair1);
    maps.push(pair2);
    scope = make_pair(15, 35);
    maps.trimByScope(scope);
    TS_ASSERT_EQUALS(maps.size(), 2);
    TS_ASSERT_EQUALS(maps[0].first, 15);
    TS_ASSERT_EQUALS(maps[0].second, 20);
    TS_ASSERT_EQUALS(maps[1].first, 30);
    TS_ASSERT_EQUALS(maps[1].second, 35);

    maps.clear();
    maps.push(pair1);
    maps.push(pair2);
    scope = make_pair(31, 45);
    maps.trimByScope(scope);
    TS_ASSERT_EQUALS(maps.size(), 1);
    TS_ASSERT_EQUALS(maps[0].first, 31);
    TS_ASSERT_EQUALS(maps[0].second, 40);

    maps.clear();
    maps.push(pair1);
    maps.push(pair2);
    scope = make_pair(9, 45);
    maps.trimByScope(scope);
    TS_ASSERT_EQUALS(maps.size(), 2);
    TS_ASSERT_EQUALS(maps[0].first, 10);
    TS_ASSERT_EQUALS(maps[0].second, 20);
    TS_ASSERT_EQUALS(maps[1].first, 30);
    TS_ASSERT_EQUALS(maps[1].second, 40);
  }
};
