cmake_minimum_required (VERSION 2.6)
project (Med)

set(CMAKE_CXX_FLAGS "-std=c++11")

find_package(PkgConfig REQUIRED)
pkg_search_module(GTK3 REQUIRED gtk+-3.0)
pkg_search_module(JSONCPP REQUIRED jsoncpp)

add_executable(med src/med.cpp src/main.cpp)
add_executable(testfile src/test.c)
add_executable(medgui src/med.cpp src/main-gui.cpp)

add_definitions(${GTK3_CFLAGS} ${JSONCPP_CFLAGS})
target_link_libraries(med ${GTK3_LIBRARIES} ${JSONCPP_LIBRARIES} -lpthread)
target_link_libraries(medgui ${GTK3_LIBRARIES} ${JSONCPP_LIBRARIES} -lpthread)
