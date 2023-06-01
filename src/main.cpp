#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "granthalaya.h"
#include "history.h"
#include "interpret.h"
#include "readline.h"
#include "repl.h"

void load_library() {
  std::string str((char*)granthalaya_tdl, granthalaya_tdl_len);
  std::istringstream is(str);
  std::string line;

  while (std::getline(is, line)) interpret_line(line);
}

int main(int argc, char** argv) {
  bool interactive = (argc == 1);
  load_library();
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
    run_repl();
  }
}
