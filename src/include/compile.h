#include <llvm/IR/IRBuilder.h>

#include <string>

#include "common.h"
class Compiler : public Inpiler {
 public:
  Compiler(std::string s);
  ~Compiler();
  std::string handle_line(std::string str);

 private:
  llvm::Value* generate_code(ASTNode* node);
  llvm::Value* generate_arithmetic(char op, ListNode* listnode);
  llvm::Value* generate_print(ASTNode* node, char end);
  llvm::Value* generate_irnode(uint32_t type, int64_t value);
  llvm::Value* generate_irnode(uint32_t type, llvm::Value* value);
  llvm::Value* generate_irnode(ASTNode* node);
  llvm::Value* generate_define(SymbolNode* symnode, ASTNode* valuenode);
  llvm::Value* generate_isequal(ASTNode* oprnd1, ASTNode* oprnd2);
  llvm::Value* generate_istype(ASTNode* oprnd1, uint32_t oprnd2);
  llvm::Value* generate_greater(ASTNode* node1, ASTNode* node2);
  llvm::Value* generate_exit(ASTNode* node);
  llvm::Value* generate_if(ASTNode* cond, ASTNode* ifbody, ASTNode* elsebody);
  llvm::Value* generate_car(ASTNode* node);
  llvm::Value* generate_cdr(ASTNode* node);
  llvm::Value* generate_cons(ASTNode* node, ASTNode* listnode);
  llvm::Value* generate_lambda_call(ListNode* node);
  llvm::Value* generate_lambda(LambdaNode* node);
  llvm::Value* generate_exception();
  llvm::Value* generate_string(StringNode* node);
  std::string mfilename;
  llvm::Module* pmodule;
  llvm::IRBuilder<>* pbuilder;
  llvm::Function* pfun;
  llvm::StructType* irnode;
  std::unique_ptr<llvm::Module> originalModule;
  llvm::Value* mret;
  llvm::Function* mainFun;
};
