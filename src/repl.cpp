#include <iostream>

#include "common.h"
#include "history.h"
#include "interpret.h"
#include "readline.h"
int run_repl() {
  char* line;
  const char* green_prompt = "\u001b[32mtodalu>\u001b[0m ";
  const char* red_prompt = "\u001b[31mtodalu>\u001b[0m ";
  const char* curr_prompt = green_prompt;
  Interpreter engine;
  while ((line = readline(curr_prompt)) != nullptr) {
    curr_prompt = red_prompt;
    try {
      auto res = engine.handle_line(line);
      if (!res.empty()) std::cout << "=> " << res;
      curr_prompt = green_prompt;
    } catch (TodaluException& e) {
      std::cerr << "Parse error: " << e.what() << std::endl;
    } catch (std::exception& e) {
      std::cerr << "Unknown error when interpreting line : " << e.what()
                << std::endl;
    }
    if (*line) add_history(line);
    free(line);
  }
  return 0;
}
