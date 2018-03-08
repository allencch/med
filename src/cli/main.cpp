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

#define COMMAND_SCAN 1
#define COMMAND_FILTER 2
#define COMMAND_LIST 3

using namespace std;

int const PROMPT_BUFFER = 255;

pid_t g_pid;
MemScanner* scanner;

int interpretCommand(const string& command) {
  if (command == "s") return COMMAND_SCAN;
  else if (command == "f") return COMMAND_FILTER;
  return COMMAND_LIST;
}

void scan(const string& value) {
  int v = stol(value);
  vector<MemPtr> mems = scanner->scan((Byte*)&v, 4, "int32", ScanParser::OpType::Eq);

  for (size_t i = 0; i < mems.size(); i++) {
    mems[i]->dump();
  }
}

void filter(const string& value) {}
void showList() {}

void interpretLine(string command) {
  vector<string> splitted = StringUtil::split(command, ' ');
  int cmd = interpretCommand(splitted[0]);
  if (cmd == COMMAND_SCAN) {
    scan(splitted[1]);
  }
  else if (cmd == COMMAND_FILTER) {
    filter(splitted[2]);
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
  scanner = new MemScanner(g_pid);

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
  delete scanner;

  return 0;
}
