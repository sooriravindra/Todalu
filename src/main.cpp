#include <getopt.h>

#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>

#include "compile.h"
#include "granthalaya.h"
#include "history.h"
#include "interpret.h"
#include "readline.h"
#include "repl.h"

bool is_balanced(std::string& line) {
  int count = 0;
  for (auto c : line) {
    if (c == '(')
      count++;
    else if (c == ')')
      count--;
  }
  return (count <= 0);
}

template <typename stream_type>
void process_stream(stream_type& is, bool compile) {
  std::string line;
  std::string wholeline = "";
  while (std::getline(is, line)) {
    wholeline += line;
    if (is_balanced(wholeline)) {
      if (compile)
        compile_line(wholeline);
      else
        interpret_line(wholeline);
      wholeline = "";
    }
  }
}

void load_library() {
  std::string str((char*)granthalaya_tdl, granthalaya_tdl_len);
  std::istringstream is(str);
  process_stream(is, false);
}

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

  if (!compile) {
    load_library();
  }
  if (interactive) {
    run_repl();
  } else {
    std::ifstream fs(filename);
    if (!fs.good()) {
      std::cerr << "Input file couldn't be read" << std::endl;
    }
    process_stream(fs, compile);
  }
  free_env();
}
