#ifndef THREAD_MANAGER
#define THREAD_MANAGER

#include <functional>
#include <vector>
#include <future>

const int TM_SLEEP_DURATION = 30; // milliseconds

typedef std::function<void()> TMTask;

class ThreadManager {
public:
  ThreadManager(int maxThread = 4);
  virtual ~ThreadManager();

  void queueTask(TMTask* fn);
  void start();

private:
  std::vector<TMTask*> container;
  int maxThreads;
  int numOfRunningThreads;
  int currThreadIndex;

  std::future<void> startTask(int index);
};

#endif
