SOURCES += ../src/med.cpp \
	   ../src/main-qt.cpp
#RESOURCES += ../main_qt.qrc
HEADERS += ../src/med.hpp

QT += core gui uitools

QMAKE_CXXFLAGS += -std=c++11 `pkg-config --cflags jsoncpp`
QMAKE_LFLAGS += `pkg-config --libs jsoncpp`
#QMAKE_CFLAGS += -std=c++11
#CONFIG += c++11

TARGET = med-qt
