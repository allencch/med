cmake_minimum_required(VERSION 3.12)
project(MedRewrite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Og -Wl,-export-dynamic")

include_directories(include)

# Find dependencies
find_package(PkgConfig REQUIRED)
# pkg_search_module(JSONCPP REQUIRED jsoncpp) # Not used yet in new core
# pkg_search_module(ICU REQUIRED icu-uc) # Not used yet in new core

# Core Library
file(GLOB med_SRC "src/med/*.cpp")
add_library(med_core SHARED ${med_SRC})
target_link_libraries(med_core pthread)

# Testing
find_package(CxxTest)
if(CXXTEST_FOUND)
    enable_testing()
    include_directories(${CXXTEST_INCLUDE_DIR})
    
    CXXTEST_ADD_TEST(TestPhase1 TestPhase1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/TestPhase1.hpp)
    target_link_libraries(TestPhase1 med_core)

    CXXTEST_ADD_TEST(TestPhase2 TestPhase2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/TestPhase2.hpp)
    target_link_libraries(TestPhase2 med_core)
endif()
