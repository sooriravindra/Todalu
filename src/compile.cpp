#include "compile.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

#include "ast.h"
#include "common.h"

llvm::LLVMContext context;
using namespace llvm;

Compiler::Compiler(std::string s) : mfilename(s) {
  pmodule = new Module(mfilename, context);
  pbuilder = new IRBuilder<>(context);
  irnode = StructType::create(context, "IRNode");
  irnode->setBody(pbuilder->getInt8Ty(), pbuilder->getInt64Ty());
  FunctionType* mainFunType = FunctionType::get(pbuilder->getInt32Ty(), false);
  Function* mainFun = Function::Create(mainFunType, Function::ExternalLinkage, "main", *pmodule);
  BasicBlock* mainEntry = BasicBlock::Create(context, "entry", mainFun);
  pbuilder->SetInsertPoint(mainEntry);
}

Value* Compiler::generate_code(ASTNode* node) {
  switch (node->type()) {
  case ASTNodeType::List: {
    auto it = dynamic_cast<ListNode*>(node)->list.begin();
    auto itend = dynamic_cast<ListNode*>(node)->list.end();
    while (it != itend)
      generate_code(*it);
    break;
  }
  case ASTNodeType::Lambda:
  case ASTNodeType::String:
    throw std::runtime_error("Not implemented");
  default: {
    AllocaInst* nodeobj = pbuilder->CreateAlloca(irnode, nullptr, "node");
    Value* typeidx = pbuilder->getInt8(0);
    Value* fieldtype = pbuilder->CreateGEP(irnode, nodeobj, typeidx, "field-type");
    Value* valueidx = pbuilder->getInt8(1);
    Value* fieldvalue = pbuilder->CreateGEP(irnode, nodeobj, valueidx, "field-value");

    pbuilder->CreateStore(pbuilder->getInt8(node->type()),fieldtype);
    switch(node->type()) {
    case ASTNodeType::Bool:
      pbuilder->CreateStore(pbuilder->getInt64(dynamic_cast<BoolNode*>(node)->getBool()? 1 : 0), fieldvalue);
    case ASTNodeType::Integer:
      pbuilder->CreateStore(pbuilder->getInt64(dynamic_cast<IntegerNode*>(node)->value), fieldvalue);
    case ASTNodeType::Decimal:
      pbuilder->CreateStore(pbuilder->getInt64(dynamic_cast<IntegerNode*>(node)->value), fieldvalue);
    default:
      throw std::runtime_error("Not implemented");
    }
    return nodeobj;
  }
  }
}
std::string Compiler::handle_line(std::string line) {
  std::string success = "";
  if (is_comment(line)) return success;
  auto tokens = tokenizer(line);
  auto ast = create_ast(tokens);

  if (ast.size() == 0) return success;
  if (ast.size() != 1)
    throw TodaluException("Contains more than one node at the base");

  generate_code(ast.front());

  return success;
}

Compiler::~Compiler() {
  pbuilder->CreateRet(pbuilder->getInt32(46));
  verifyModule(*pmodule, &outs());
  pmodule->print(outs(), nullptr);
}
