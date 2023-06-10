#include "interpret.h"

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>

#include "ast.h"
#include "common.h"
#include "eval.h"

std::string Interpreter::handle_line(std::string str) {
  if (is_comment(str)) return "";
  auto tokens = tokenizer(str);
  auto ast = create_ast(tokens);

  if (ast.size() == 0) return "";

  if (ast.size() != 1)
    throw TodaluException("Contains more than one node at the base");

  std::unique_ptr<ASTNode> result(eval_tree(ast.front()));

  free_ast(ast);

  return result->getRepr() + "\n";
}

Interpreter::~Interpreter() { free_env(); }
