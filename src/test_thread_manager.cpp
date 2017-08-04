#include <iostream>
#include <chrono>
#include <future>

#include "med/ThreadManager.hpp"

using namespace std;

int main() {
  cout << "hello" << endl;
  ThreadManager tm(2);
  TMTask fn = []() {
    for (int i = 0; i < 4; i++) {
      this_thread::sleep_for(chrono::milliseconds(300));
      cout << "thread1: " << i << endl;
    }
  };
  TMTask fn2 = []() {
    for (int i = 0; i < 8; i++) {
      this_thread::sleep_for(chrono::milliseconds(200));
      cout << "thread2: " << i << endl;
    }
  };

  TMTask fn3 = []() {
    for(int i = 0; i < 10; i++) {
      this_thread::sleep_for(chrono::milliseconds(100));
      cout << "thread3: " << i << endl;
    }
  };
  TMTask fn4 = []() {
    for(int i = 0; i < 20; i++) {
      this_thread::sleep_for(chrono::milliseconds(200));
      cout << "thread4: " << i << endl;
    }
  };
  TMTask fn5 = []() {
    for(int i = 0; i < 10; i++) {
      this_thread::sleep_for(chrono::milliseconds(300));
      cout << "thread5: " << i << endl;
    }
  };

  tm.queueTask(&fn);
  tm.queueTask(&fn2);
  tm.queueTask(&fn3);
  tm.queueTask(&fn4);
  tm.queueTask(&fn5);
  tm.start();
  // tm.clear();
  this_thread::sleep_for(chrono::seconds(5));

  return 0;
}
