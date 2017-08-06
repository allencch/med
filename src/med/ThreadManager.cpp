#include <future>
#include <string>
#include <iostream>
#include <condition_variable>
#include <mutex>
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

void ThreadManager::clear() {
  for(auto task : container) {
    delete task;
  }
  container.clear();
}

void ThreadManager::start() {
  // Note: Cannot assign startTask() to a variable then push_back to vector
  // The future must be destroyed at the end, else the asynchronous cannot work.
  // That is why, store all the futures to the local variable, which will be destroyed at the end of function.
  currThreadIndex = 0;
  vector<future<void>> futures;

  unique_lock<mutex> lk(mut);
  while (currThreadIndex < (int)container.size()) {
    if (numOfRunningThreads >= maxThreads) {
      cv.wait(lk, [this] {
          return numOfRunningThreads < maxThreads;
        });
    }
    numOfRunningThreads++;
    futures.push_back(startTask(currThreadIndex));
    currThreadIndex++;
  }
}

future<void> ThreadManager::startTask(int index) {
  // Note: Cannot capture by reference, because the "index" will be overwritten during async
  future<void> fut = async([index, this]() {
      if (index < 0 || index >= (int)container.size()) {
        throw "Task index out of range " + to_string(index);
      }
      TMTask* fn = container[index];
      (*fn)();
      numOfRunningThreads--;

      cv.notify_one();
    });
  return fut;
}
