cmake_minimum_required (VERSION 2.8.11)
project (Med)

cmake_policy(SET CMP0071 NEW) # 3.10.1 introduced policy, "cmake --help-policy CMP0071" for detail.

# set(CMAKE_CXX_FLAGS "-std=c++17")
set(CMAKE_CXX_FLAGS "-std=c++17 -g -Og") # for strong debugging purpose
# set(CMAKE_CXX_FLAGS "-std=c++17 -g") # for debugging
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-export-dynamic")
set(CMAKE_INCLUDE_CURRENT_DIR ON) #required for qt moc
set(CMAKE_AUTOMOC ON) #automatic moc

include_directories(./include)

find_package(PkgConfig REQUIRED)
find_package(Qt5UiTools)
pkg_search_module(JSONCPP REQUIRED jsoncpp)
pkg_search_module(ICU REQUIRED icu-uc)

add_definitions(${JSONCPP_CFLAGS} ${ICU_CFLAGS})

add_executable(testfile src/testfile.cpp)

file(GLOB med_SRC "src/med/*.cpp")
file(GLOB mem_SRC "src/mem/*.cpp")
add_library(med SHARED ${med_SRC} ${mem_SRC})
target_link_libraries(med ${JSONCPP_LIBRARIES} ${ICU_LIBRARIES} -lpthread)

# Because the header files are located in different location,
# and these header files use Q_OBJECT, so need to generate
# moc. And create it as a static library.
# To generate moc,
# qt5_generate_moc(src/gui/med-qt.cpp med-qt.moc)
# But using automatic moc can solve the Q_OBJECT issue in other cpp files

# med-ui
file(GLOB ui_HEADER "include/ui/*.hpp")
qt5_wrap_cpp(moc_med-ui ${ui_HEADER})
add_library(moc-med-ui ${moc_med-ui})
qt5_use_modules(moc-med-ui Widgets)
set_property(SOURCE ${ui_HEADER} PROPERTY SKIP_AUTOMOC ON)

file(GLOB ui_SRC "src/ui/*.cpp")
add_executable(med-ui ${ui_SRC})
target_link_libraries(med-ui moc-med-ui med Qt5::UiTools)


# CLI
file(GLOB cli_SRC "src/cli/*.cpp")
add_executable(med-cli ${cli_SRC})
target_link_libraries(med-cli med readline)

# Executable test files
add_executable(test_thread_manager src/med/ThreadManager.cpp src/test_thread_manager.cpp)
target_link_libraries(test_thread_manager -lpthread)

add_executable(test_memio src/test_memio.cpp)
target_link_libraries(test_memio med)

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

  CXXTEST_ADD_TEST(testStringUtil testStringUtil.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/StringUtil.hpp)
  target_link_libraries(testStringUtil med)

  CXXTEST_ADD_TEST(testMedCommon testMedCommon.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/MedCommon.hpp)
  target_link_libraries(testMedCommon med)

  CXXTEST_ADD_TEST(testCoder testCoder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/Coder.hpp)
  target_link_libraries(testCoder med)

  CXXTEST_ADD_TEST(testMemIO testMemIO.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/MemIO.hpp)
  target_link_libraries(testMemIO med)

  CXXTEST_ADD_TEST(testMem testMem.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/Mem.hpp)
  target_link_libraries(testMem med)

  CXXTEST_ADD_TEST(testPem testPem.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/Pem.hpp)
  target_link_libraries(testPem med)

  CXXTEST_ADD_TEST(testMemScanner testMemScanner.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/MemScanner.hpp)
  target_link_libraries(testMemScanner med)

  file(GLOB test_HEADER "tests/*.hpp")
  set_property(SOURCE ${gui_HEADER} PROPERTY SKIP_AUTOMOC ON)
endif()

install(TARGETS med med-ui
  RUNTIME DESTINATION /opt/med
  LIBRARY DESTINATION lib)
install(FILES ui/main-qt.ui ui/process.ui ui/mem-editor.ui
  DESTINATION /opt/med)
install(FILES med DESTINATION bin
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
