#include <string>
#include "common.h"
class Compiler : public Inpiler {
public:
  std::string handle_line(std::string str);
};
