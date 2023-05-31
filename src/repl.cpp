#include <fstream>
#include <iostream>
#include <string>

#include "history.h"
#include "interpret.h"
#include "readline.h"

int main(int argc, char** argv) {
  char* line;
  bool interactive = (argc == 1);
  const char* green_prompt = "\u001b[32mtodalu>\u001b[0m ";
  const char* red_prompt = "\u001b[31mtodalu>\u001b[0m ";
  const char* curr_prompt = green_prompt;

  if (not interactive) {
    std::ifstream f(argv[1]);
    if (!f.good()) {
      std::cerr << "Input file couldn't be read";
    }
    std::string source;
    while (std::getline(f, source)) {
      std::cout << interpret_line(source);
    }
  } else {
    while ((line = readline(curr_prompt)) != nullptr) {
      curr_prompt = red_prompt;
      try {
        std::cout << interpret_line(line);
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
}
