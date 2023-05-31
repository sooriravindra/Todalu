#include <iostream>

#include "interpret.h"
#include "linenoise.h"

int main(int argc, char** argv) {
  char* line;
  bool interactive = (argc == 1);
  const char* green_prompt = "\e[0;32mtodalu>\e[0m ";
  const char* red_prompt = "\e[0;31mtodalu>\e[0m ";
  const char* curr_prompt = green_prompt;

  if (not interactive) {
    // TODO Read file and interpret each line
  } else {
    while ((line = linenoise(curr_prompt)) != nullptr) {
      curr_prompt = red_prompt;
      try {
        std::cout << interpret_line(line) << std::endl;
        curr_prompt = green_prompt;
      } catch (ParseError& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
      } catch (std::exception& e) {
        std::cerr << "Unknown error when interpreting line : " << e.what()
                  << std::endl;
      }
      linenoiseFree(line);
    }
  }
}
