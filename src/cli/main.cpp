#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <readline/readline.h>
#include <readline/history.h>

// For segment fault tracing
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>

#include "mem/StringUtil.hpp"
#include "mem/MemScanner.hpp"
#include "mem/MemManager.hpp"
#include "mem/MemEd.hpp"

#define COMMAND_SCAN 1
#define COMMAND_FILTER 2
#define COMMAND_LIST 3

using namespace std;

int const PROMPT_BUFFER = 255;

pid_t g_pid;
MemEd* memed;

int interpretCommand(const string& command) {
  if (command == "s") return COMMAND_SCAN;
  else if (command == "f") return COMMAND_FILTER;
  return COMMAND_LIST;
}

void scan(const string& value) {
  vector<MemPtr> mems = memed->scan(value, "int32");
  printf("Scanned %zu\n", mems.size());
}

void filter(const string& value) {
  vector<MemPtr> mems = memed->filter(value, "int32");
  printf("Filtered %zu\n", mems.size());
}

void showList() {
  auto scans = memed->getScans();
  for (size_t i = 0; i < scans.size(); i++) {
    cout << scans.getAddress(i) << "\t";
    scans.dump(i, false);
    cout << scans.getValue(i) << endl;
  }
}

void interpretLine(string command) {
  vector<string> splitted = StringUtil::split(command, ' ');

  if (splitted.size() == 0) return;

  int cmd = interpretCommand(splitted[0]);
  if (cmd == COMMAND_SCAN) {
    scan(splitted[1]);
  }
  else if (cmd == COMMAND_FILTER) {
    filter(splitted[1]);
  }
  else {
    showList();
  }
}

void handler(int sig) {
  void* array[10];
  size_t size;

  size = backtrace(array, 10);

  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, char** argv) {
  if (argc < 2) {
    cerr << "Missing argument\n"
      "Usage: med-cli [pid]" << endl;
    return -1;
  }
  signal(SIGSEGV, handler);

  g_pid = stol(string(argv[1]));
  memed = new MemEd(g_pid);

  char shellPrompt[PROMPT_BUFFER];
  cout << "Med CLI" <<endl;
  rl_bind_key('\t', rl_complete);
  for (;;) {
    snprintf(shellPrompt, sizeof(shellPrompt), "> ");
    char* input = readline(shellPrompt);

    if (!input) break;
    add_history(input);

    interpretLine(string(input));

    free(input);
  }
  delete memed;

  return 0;
}
