#include <thread>
#include <future>
#include <string>
#include <iostream>
#include <chrono>
#include "med/ThreadManager.hpp"

using namespace std;

ThreadManager::ThreadManager(int maxThreads) {
  this->maxThreads = maxThreads;
  this->numOfRunningThreads = 0;
  this->currThreadIndex = -1;
}

ThreadManager::~ThreadManager() {}

void ThreadManager::queueTask(TMTask* fn) {
  container.push_back(fn);
}

void ThreadManager::start() {
  // Note: Cannot assign startTask() to a variable then push_back to vector
  // The future must be destroyed at the end, else the asynchronous cannot work.
  // That is why, store all the futures to the local variable, which will be destroyed at the end of function.
  currThreadIndex = 0;
  vector<future<void>> futures;
  while (currThreadIndex < (int)container.size()) {
    if (numOfRunningThreads >= maxThreads) {
      this_thread::sleep_for(chrono::milliseconds(TM_SLEEP_DURATION));
      continue;
    }
    numOfRunningThreads++;
    futures.push_back(startTask(currThreadIndex));
    currThreadIndex++;
  }
}

future<void> ThreadManager::startTask(int index) {
  // Note: Cannot capture by reference, because the "index" will be overwritten during async
  ThreadManager* me = this;
  future<void> fut = async([index, me]() { 
      if (index < 0 || index >= (int)me->container.size()) {
        throw "Task index out of range " + to_string(index);
      }
      TMTask* fn = me->container[index];
      (*fn)();
      me->numOfRunningThreads--;
    });
  return fut;
}
