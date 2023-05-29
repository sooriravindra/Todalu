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
      std::cout << interpret_line(line) << std::endl;
      linenoiseFree(line);
    }
  }
}
