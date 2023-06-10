#include <string>

#include "ast.h"
#include "common.h"

class Interpreter : public Inpiler {
 public:
  ~Interpreter();
  std::string handle_line(std::string str);
};
