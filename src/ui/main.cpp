// For segment fault tracing
#include <iostream>
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>
#include <QApplication>

#include "med/MedException.hpp"
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

void print_exception(const std::exception& e, int level = 0) {
  std::cerr << std::string(level, ' ') << "exception: " << e.what() << endl;
  try {
    std::rethrow_if_nested(e);
  } catch (const std::exception& e) {
    print_exception(e, level + 1);
  } catch (...) {}
}

int main(int argc, char** argv) {
  signal(SIGSEGV, handler);
  try {
    QApplication app(argc, argv);
    new MedUi(&app);
    return app.exec();
  } catch(MedException &ex) {
    cerr << ex.getMessage() << endl;
  } catch(const std::exception& e) {
    print_exception(e);
    return 1;
  }
}
