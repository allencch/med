cmake_minimum_required (VERSION 2.8.11)
project (Med)

set(CMAKE_CXX_FLAGS "-std=c++14")
set(CMAKE_INCLUDE_CURRENT_DIR ON) #required for qt moc
set(CMAKE_AUTOMOC ON) #automatic moc

include_directories(./include)

add_definitions(${JSONCPP_CFLAGS})

find_package(PkgConfig REQUIRED)
find_package(Qt5UiTools)
pkg_search_module(JSONCPP REQUIRED jsoncpp)

add_executable(testfile src/test.c)

add_library(med SHARED
  src/med/med.cpp
  src/med/ScanParser.cpp
  src/med/MemOperator.cpp
  src/med/ThreadManager.cpp
  src/med/MedScan.cpp
  src/med/MedAddress.cpp)
target_link_libraries(med ${JSONCPP_LIBRARIES} -lpthread)

# Because the header files are located in different location,
# and these header files use Q_OBJECT, so need to generate
# moc. And create it as a static library.
qt5_wrap_cpp(moc_med-qt
  include/gui/med-qt.hpp
  include/gui/TreeModel.hpp
  include/gui/ComboBoxDelegate.hpp
  include/gui/StoreTreeModel.hpp
  include/gui/CheckBoxDelegate.hpp
  include/gui/MemEditor.hpp
  include/gui/MemEditorEventListener.hpp)
add_library(moc-med-qt ${moc_med-qt})
qt5_use_modules(moc-med-qt Widgets)

# qt5_generate_moc(src/gui/med-qt.cpp med-qt.moc) #Using automatic moc can solve the Q_OBJECT issue in other cpp files
add_executable(med-qt
  src/gui/med-qt.cpp
  src/gui/TreeItem.cpp
  src/gui/TreeModel.cpp
  src/gui/ComboBoxDelegate.cpp
  src/gui/StoreTreeModel.cpp
  src/gui/CheckBoxDelegate.cpp
  src/gui/MemEditor.cpp
  src/gui/MemEditorEventListener.cpp)
target_link_libraries(med-qt moc-med-qt med Qt5::UiTools)

add_executable(test_thread_manager src/med/ThreadManager.cpp src/test_thread_manager.cpp)
target_link_libraries(test_thread_manager -lpthread)

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
endif()

install(TARGETS med med-qt
  RUNTIME DESTINATION /opt/med
  LIBRARY DESTINATION lib)
install(FILES ui/main-qt.ui ui/process.ui ui/mem-editor.ui
  DESTINATION /opt/med)
install(FILES med DESTINATION bin
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
