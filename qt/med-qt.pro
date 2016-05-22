SOURCES += ../med.cpp \
	   ../main-qt.cpp 
#RESOURCES += ../main_qt.qrc
HEADERS += ../med.h

QT += core gui uitools

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CFLAGS += -std=c++11
#CONFIG += c++11

TARGET = med-qt
