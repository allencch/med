cmake_minimum_required (VERSION 2.8.11)
project (Med)

set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_INCLUDE_CURRENT_DIR ON) #required for qt moc
#set(CMAKE_AUTOMOC ON) #automatic moc

find_package(PkgConfig REQUIRED)
find_package(Qt5UiTools)
pkg_search_module(GTK3 REQUIRED gtk+-3.0)
pkg_search_module(JSONCPP REQUIRED jsoncpp)

add_executable(med src/med.cpp src/main.cpp)
add_executable(testfile src/test.c)
add_executable(medgui src/med.cpp src/main-gui.cpp)

qt5_generate_moc(src/main-qt.cpp main-qt.moc)
add_executable(med-qt src/med.cpp src/main-qt.cpp main-qt.moc)

add_definitions(${GTK3_CFLAGS} ${JSONCPP_CFLAGS})
target_link_libraries(med ${GTK3_LIBRARIES} ${JSONCPP_LIBRARIES} -lpthread)
target_link_libraries(medgui ${GTK3_LIBRARIES} ${JSONCPP_LIBRARIES} -lpthread)

target_link_libraries(med-qt ${JSONCPP_LIBRARIES} -lpthread Qt5::UiTools)
