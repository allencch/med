cmake_minimum_required (VERSION 2.8.11)
project (Med)

cmake_policy(SET CMP0071 NEW) # 3.10.1 introduced policy, "cmake --help-policy CMP0071" for detail.

# set(CMAKE_CXX_FLAGS "-std=c++14")
set(CMAKE_CXX_FLAGS "-std=c++14 -g") # for debugging purpose
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-export-dynamic")
set(CMAKE_INCLUDE_CURRENT_DIR ON) #required for qt moc
set(CMAKE_AUTOMOC ON) #automatic moc

include_directories(./include)

find_package(PkgConfig REQUIRED)
find_package(Qt5UiTools)
pkg_search_module(JSONCPP REQUIRED jsoncpp)
pkg_search_module(ICU REQUIRED icu-uc)

add_definitions(${JSONCPP_CFLAGS} ${ICU_CFLAGS})

add_executable(testfile src/test.c)

file(GLOB med_SRC "src/med/*.cpp")
file(GLOB mem_SRC "src/mem/*.cpp")
add_library(med SHARED ${med_SRC} ${mem_SRC})
target_link_libraries(med ${JSONCPP_LIBRARIES} ${ICU_LIBRARIES} -lpthread)

# Because the header files are located in different location,
# and these header files use Q_OBJECT, so need to generate
# moc. And create it as a static library.
file(GLOB gui_HEADER "include/gui/*.hpp")
qt5_wrap_cpp(moc_med-qt ${gui_HEADER})
add_library(moc-med-qt ${moc_med-qt})
qt5_use_modules(moc-med-qt Widgets)

set_property(SOURCE ${gui_HEADER} PROPERTY SKIP_AUTOMOC ON)

# qt5_generate_moc(src/gui/med-qt.cpp med-qt.moc) #Using automatic moc can solve the Q_OBJECT issue in other cpp files
file(GLOB gui_SRC "src/gui/*.cpp")
add_executable(med-qt ${gui_SRC})
target_link_libraries(med-qt moc-med-qt med Qt5::UiTools)

add_executable(test_thread_manager src/med/ThreadManager.cpp src/test_thread_manager.cpp)
target_link_libraries(test_thread_manager -lpthread)

add_executable(test_snapshot src/test_snapshot.cpp)
target_link_libraries(test_snapshot med)

add_executable(test_byte_manager src/test_byte_manager.cpp)
target_link_libraries(test_byte_manager med)

#https://cmake.org/cmake/help/v3.7/module/FindCxxTest.html
find_package(CxxTest)
if(CXXTEST_FOUND)
  include_directories(${CXXTEST_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/)
  enable_testing()
  CXXTEST_ADD_TEST(testMemOperator testMemOperator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/MemOperator.hpp)
  target_link_libraries(testMemOperator med)

  CXXTEST_ADD_TEST(testScanParser testScanParser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/ScanParser.hpp)
  target_link_libraries(testScanParser med)

  CXXTEST_ADD_TEST(testCoder testCoder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/Coder.hpp)
  target_link_libraries(testCoder med)

  CXXTEST_ADD_TEST(testSnapshot testSnapshot.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/Snapshot.hpp)
  target_link_libraries(testSnapshot med)

  CXXTEST_ADD_TEST(testSnapshotScan testSnapshotScan.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/SnapshotScan.hpp)
  target_link_libraries(testSnapshotScan med)

  CXXTEST_ADD_TEST(testMemIO testMemIO.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/MemIO.hpp)
  target_link_libraries(testMemIO med)

  CXXTEST_ADD_TEST(testMem testMem.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/Mem.hpp)
  target_link_libraries(testMem med)

  file(GLOB test_HEADER "tests/*.hpp")
  set_property(SOURCE ${gui_HEADER} PROPERTY SKIP_AUTOMOC ON)
endif()

install(TARGETS med med-qt
  RUNTIME DESTINATION /opt/med
  LIBRARY DESTINATION lib)
install(FILES ui/main-qt.ui ui/process.ui ui/mem-editor.ui
  DESTINATION /opt/med)
install(FILES med DESTINATION bin
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
