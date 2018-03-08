#include <iostream>
#include <cstdio>
#include <string>
#include <readline/readline.h>
#include <readline/history.h>

using namespace std;

int const PROMPT_BUFFER = 255;

int main(int argc, char** argv) {
  cout << "Med CLI" <<endl;
  
  char shellPrompt[PROMPT_BUFFER];

  rl_bind_key('\t', rl_complete);

  for (;;) {
    snprintf(shellPrompt, sizeof(shellPrompt), "> ");
    char* input = readline(shellPrompt);

    if (!input) break;

    add_history(input);

    free(input);
  }

  return 0;
}
