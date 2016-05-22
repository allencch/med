CC = gcc
CXX = g++
CF = -std=c++11

all: med testfile medgui

testgui: all
	./medgui

test: all
	./med

med: med.o main.o
	$(CXX) $(CF) -o $@ $^

med.o: med.cpp
	$(CXX) $(CF) -c -o $@ $?

main.o: main.cpp
	$(CXX) $(CF) -c -o $@ $?


testfile: test.c
	$(CC) -o $@ $?

medgui: med.o main-gui.o
	$(CXX) $(CF) `pkg-config --libs gtk+-3.0 jsoncpp` -o $@ $^

main-gui.o: main-gui.cpp
	$(CXX) $(CF) `pkg-config --cflags gtk+-3.0 jsoncpp` -c -o $@ $?

