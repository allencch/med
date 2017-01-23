cmake_minimum_required (VERSION 2.8.11)
project (Med)

set(CMAKE_CXX_FLAGS "-std=c++14")
set(CMAKE_INCLUDE_CURRENT_DIR ON) #required for qt moc
set(CMAKE_AUTOMOC ON) #automatic moc

find_package(PkgConfig REQUIRED)
find_package(Qt5UiTools)
pkg_search_module(JSONCPP REQUIRED jsoncpp)

add_executable(testfile src/test.c)

#qt5_generate_moc(src/med-qt.cpp med-qt.moc) #Using automatic moc can solve the Q_OBJECT issue in other cpp files
add_executable(
  med-qt
  src/med.cpp
  src/med-qt.cpp
  src/TreeItem.cpp
  src/TreeModel.cpp
  src/ComboBoxDelegate.cpp
  src/StoreTreeModel.cpp
  src/CheckBoxDelegate.cpp)

add_definitions(${JSONCPP_CFLAGS})
target_link_libraries(med-qt ${JSONCPP_LIBRARIES} -lpthread Qt5::UiTools)
