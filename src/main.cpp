#include <getopt.h>

#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>

#include "compile.h"
#include "history.h"
#include "interpret.h"
#include "readline.h"
#include "repl.h"


int main(int argc, char** argv) {
  std::string usage = std::string("Usage :") + argv[0] +
                      " [-h] [-c] [file]\n-c to compile\n-h to print help";
  int option;
  bool compile = false;
  bool interactive = true;

  std::string filename;
  while ((option = getopt(argc, argv, "ch")) != -1) {
    switch (option) {
      case 'c':
        compile = true;
        break;
      case 'h':
        std::cout << usage << std::endl;
        return 0;
      default:
        std::cerr << "Invalid command" << std::endl;
        std::cerr << usage << std::endl;
        return 1;
    }
  }

  if (optind == argc - 1) {
    filename = argv[optind];
    interactive = false;
  } else if (optind < argc) {
    std::cerr << usage << std::endl;
    return 1;
  }

  if (interactive) {
    return run_repl();
  }

  // Compile or interpret filename given.
  Inpiler * engine = compile? (Inpiler *) new Compiler() : (Inpiler *) new Interpreter();
  engine->load_granthalaya();
  std::ifstream fs(filename);
  if (!fs.good()) {
      std::cerr << "Input file couldn't be read" << std::endl;
  }
  std::string line;
  std::string wholeline = "";
  while (std::getline(fs, line)) {
    wholeline += line;
    if (engine->is_balanced(wholeline)) {
      engine->handle_line(wholeline);
      wholeline = "";
    }
  }
}
