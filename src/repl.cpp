#include <iostream>

#include "interpret.h"
#include "linenoise.h"

int main(int argc, char** argv) {
  char* line;
  bool interactive = (argc == 1);

  if (not interactive) {
    // TODO Read file and interpret each line
  } else {
    while ((line = linenoise("todalu> ")) != nullptr) {
      try {
        std::cout << interpret_line(line) << std::endl;
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
