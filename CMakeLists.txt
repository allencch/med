cmake_minimum_required (VERSION 3.12.4)
project (Med)

cmake_policy(SET CMP0071 NEW) # 3.10.1 introduced policy, "cmake --help-policy CMP0071" for detail.

set(CMAKE_CXX_FLAGS "-std=c++17 -g -Og") # for strong debugging purpose
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-export-dynamic")

set(CMAKE_INCLUDE_CURRENT_DIR ON) #required for qt moc
set(CMAKE_AUTOMOC ON) #automatic moc

include_directories(./include)

find_package(PkgConfig REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
find_package(Qt6UiTools)
pkg_search_module(JSONCPP REQUIRED jsoncpp)
pkg_search_module(ICU REQUIRED icu-uc)

add_definitions(${JSONCPP_CFLAGS} ${ICU_CFLAGS})

# Disable C++ API deprecated in Qt 5.15
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x050F00)

add_executable(testfile src/testfile.cpp)

# NOTE: If adding new source file, this file needs to be "touch"-ed
file(GLOB med_SRC "src/med/*.cpp")
file(GLOB mem_SRC "src/mem/*.cpp")
add_library(mem_ed SHARED ${med_SRC} ${mem_SRC})
target_link_libraries(mem_ed ${JSONCPP_LIBRARIES} ${ICU_LIBRARIES} -lpthread)

# Because the header files are located in different location,
# and these header files use Q_OBJECT, so need to generate
# moc. And create it as a static library.
# To generate moc,
# qt5_generate_moc(src/gui/med-qt.cpp med-qt.moc)
# But using automatic moc can solve the Q_OBJECT issue in other cpp files

# med-ui
file(GLOB ui_HEADER "include/ui/*.hpp")
file(GLOB ui_SRC "src/ui/*.cpp")
# header is necessary, https://github.com/bincrafters/community/issues/466#issuecomment-426265654
add_executable(med-ui ${ui_SRC} ${ui_HEADER})
target_link_libraries(med-ui mem_ed Qt6::Widgets Qt6::UiTools)


# CLI
file(GLOB cli_SRC "src/cli/*.cpp")
add_executable(med-cli ${cli_SRC})
target_link_libraries(med-cli mem_ed readline)

# Executable test files
add_executable(test_thread_manager src/med/ThreadManager.cpp src/test_thread_manager.cpp)
target_link_libraries(test_thread_manager -lpthread)

add_executable(test_memio src/test_memio.cpp)
target_link_libraries(test_memio mem_ed)

#https://cmake.org/cmake/help/v3.7/module/FindCxxTest.html
find_package(CxxTest)
if(CXXTEST_FOUND)
  include_directories(${CXXTEST_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/)
  enable_testing()
  CXXTEST_ADD_TEST(testMemOperator testMemOperator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/MemOperator.hpp)
  target_link_libraries(testMemOperator mem_ed)

  CXXTEST_ADD_TEST(testScanParser testScanParser.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/ScanParser.hpp)
  target_link_libraries(testScanParser mem_ed)

  CXXTEST_ADD_TEST(testStringUtil testStringUtil.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/StringUtil.hpp)
  target_link_libraries(testStringUtil mem_ed)

  CXXTEST_ADD_TEST(testMedCommon testMedCommon.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/MedCommon.hpp)
  target_link_libraries(testMedCommon mem_ed)

  CXXTEST_ADD_TEST(testCoder testCoder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/Coder.hpp)
  target_link_libraries(testCoder mem_ed)

  CXXTEST_ADD_TEST(testMemIO testMemIO.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/MemIO.hpp)
  target_link_libraries(testMemIO mem_ed)

  CXXTEST_ADD_TEST(testMem testMem.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/Mem.hpp)
  target_link_libraries(testMem mem_ed)

  CXXTEST_ADD_TEST(testPem testPem.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/Pem.hpp)
  target_link_libraries(testPem mem_ed)

  CXXTEST_ADD_TEST(testMemScanner testMemScanner.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/MemScanner.hpp)
  target_link_libraries(testMemScanner mem_ed)

  CXXTEST_ADD_TEST(testSubCommand testSubCommand.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/SubCommand.hpp)
  target_link_libraries(testSubCommand mem_ed)

  CXXTEST_ADD_TEST(testScanCommand testScanCommand.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/ScanCommand.hpp)
  target_link_libraries(testScanCommand mem_ed)

  CXXTEST_ADD_TEST(testMaps testMaps.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/Maps.hpp)
  target_link_libraries(testMaps mem_ed)

  file(GLOB test_HEADER "tests/*.hpp")
  set_property(SOURCE ${gui_HEADER} PROPERTY SKIP_AUTOMOC ON)
endif()

find_program(CTEST_MEMORYCHECK_COMMAND NAMES valgrind)

install(TARGETS mem_ed med-ui
  RUNTIME DESTINATION /opt/med
  LIBRARY DESTINATION lib)
install(FILES ui/main-qt.ui ui/process.ui ui/mem-editor.ui
  DESTINATION /opt/med)
install(FILES med DESTINATION bin
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
