#include <string>
#include <llvm/IR/IRBuilder.h>
#include "common.h"
class Compiler : public Inpiler {
public:
  Compiler(std::string s);
  ~Compiler();
  std::string handle_line(std::string str);
private:
  llvm::Value* generate_code(ASTNode* node);
  std::string mfilename;
  llvm::Module* pmodule;
  llvm::IRBuilder<> * pbuilder;
  llvm::StructType* irnode;
};
