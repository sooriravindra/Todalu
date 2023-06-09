#include "ast.h"
#include "common.h"
#include <string>

class Interpreter : public Inpiler {
public:
  ~Interpreter();
  std::string handle_line(std::string str);
};
