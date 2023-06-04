#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>

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
void interpret_stream(stream_type& is) {
  std::string line;
  std::string wholeline = "";
  while (std::getline(is, line)) {
    wholeline += line;
    if (is_balanced(wholeline)) {
      interpret_line(wholeline);
      wholeline = "";
    }
  }
}

void load_library() {
  std::string str((char*)granthalaya_tdl, granthalaya_tdl_len);
  std::istringstream is(str);
  interpret_stream(is);
}

int main(int argc, char** argv) {
  bool interactive = (argc == 1);
  load_library();
  if (not interactive) {
    std::ifstream fs(argv[1]);
    if (!fs.good()) {
      std::cerr << "Input file couldn't be read";
    }
    interpret_stream(fs);
  } else {
    run_repl();
  }
  free_env();
}
