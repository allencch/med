cmake_minimum_required (VERSION 2.8.11)
project (Med)

set(CMAKE_CXX_FLAGS "-std=c++14")
set(CMAKE_INCLUDE_CURRENT_DIR ON) #required for qt moc
set(CMAKE_AUTOMOC ON) #automatic moc

add_definitions(${JSONCPP_CFLAGS})

find_package(PkgConfig REQUIRED)
find_package(Qt5UiTools)
pkg_search_module(JSONCPP REQUIRED jsoncpp)

add_executable(testfile src/test.c)

add_library(med SHARED
  src/med.cpp
  src/ScanParser.cpp
  src/MemOperator.cpp
  src/ThreadManager.cpp)
target_link_libraries(med ${JSONCPP_LIBRARIES} -lpthread)

#qt5_generate_moc(src/med-qt.cpp med-qt.moc) #Using automatic moc can solve the Q_OBJECT issue in other cpp files
add_executable(med-qt
  src/med-qt.cpp
  src/TreeItem.cpp
  src/TreeModel.cpp
  src/ComboBoxDelegate.cpp
  src/StoreTreeModel.cpp
  src/CheckBoxDelegate.cpp)
target_link_libraries(med-qt med Qt5::UiTools)

add_executable(test_thread_manager src/ThreadManager.cpp src/test_thread_manager.cpp)
target_link_libraries(test_thread_manager -lpthread)

#https://cmake.org/cmake/help/v3.7/module/FindCxxTest.html
find_package(CxxTest)
if(CXXTEST_FOUND)
  include_directories(${CXXTEST_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/)
  enable_testing()
  CXXTEST_ADD_TEST(med-test med-test.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/med-test.hpp)
  target_link_libraries(med-test med)
endif()

install(TARGETS med med-qt
  RUNTIME DESTINATION /opt/med
  LIBRARY DESTINATION lib)
install(FILES ui/main-qt.ui ui/process.ui
  DESTINATION /opt/med)
install(FILES med DESTINATION bin
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
