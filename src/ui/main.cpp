#include <QApplication>

// For segment fault tracing
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>

#include "ui/Ui.hpp"

using namespace std;

// https://stackoverflow.com/questions/77005/how-to-automatically-generate-a-stacktrace-when-my-gcc-c-program-crashes
void handler(int sig) {
  void* array[10];
  size_t size;

  size = backtrace(array, 10);

  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, char** argv) {
  signal(SIGSEGV, handler);
  QApplication app(argc, argv);
  new MedUi(&app);
  return app.exec();
}
