#include "compile.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "ast.h"
#include "common.h"

llvm::LLVMContext context;
llvm::Module module("todalu", context);
llvm::IRBuilder<> builder(context);

std::string Compiler::handle_line(std::string line) {
  if (is_comment(line)) return "";
  auto tokens = tokenizer(line);
  auto ast = create_ast(tokens);

  if (ast.size() == 0) return "";
  if (ast.size() != 1)
    throw TodaluException("Contains more than one node at the base");

  return "";
}
