#include <iostream>

#include "history.h"
#include "interpret.h"
#include "readline.h"
void run_repl() {
  char* line;
  const char* green_prompt = "\u001b[32mtodalu>\u001b[0m ";
  const char* red_prompt = "\u001b[31mtodalu>\u001b[0m ";
  const char* curr_prompt = green_prompt;
  while ((line = readline(curr_prompt)) != nullptr) {
    curr_prompt = red_prompt;
    try {
      auto res = interpret_line(line);
      std::cout << "=> " << res;
      curr_prompt = green_prompt;
    } catch (ParseError& e) {
      std::cerr << "Parse error: " << e.what() << std::endl;
    } catch (std::exception& e) {
      std::cerr << "Unknown error when interpreting line : " << e.what()
                << std::endl;
    }
    if (*line) add_history(line);
    free(line);
  }
}
