cmake_minimum_required(VERSION 3.16)
project(MedRewrite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Og")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(include)

# Find dependencies
find_package(PkgConfig REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets Gui UiTools REQUIRED)
pkg_search_module(JSONCPP REQUIRED jsoncpp)

include_directories(include ${JSONCPP_INCLUDE_DIRS})

# Core Library
file(GLOB med_SRC "src/med/*.cpp")
add_library(med_core SHARED ${med_SRC})
target_link_libraries(med_core pthread ${JSONCPP_LIBRARIES})

# UI
set(ui_SRC
    src/ui/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MedWorker.cpp
    src/ui/ProcessDialog.cpp
    src/ui/ComboBoxDelegate.cpp
    src/ui/CheckBoxDelegate.cpp
    include/ui/MainWindow.hpp
    include/ui/MedWorker.hpp
    include/ui/ProcessDialog.hpp
    include/ui/ComboBoxDelegate.hpp
    include/ui/CheckBoxDelegate.hpp
)
add_executable(med-ui ${ui_SRC})
target_link_libraries(med-ui med_core Qt6::Widgets Qt6::UiTools Qt6::Gui Qt6::Core)

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
