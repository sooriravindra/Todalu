#include "trt.h"

#include <cstdarg>
#include <iostream>

typedef union _as {
  int64_t integer;
  double decimal;
} as;

enum ASTNodeType { Bool = 0, Integer, Decimal, Symbol, List, Lambda, String };
IRNode* add(uint32_t num_args, ...) {
  va_list args;
  va_start(args, num_args);
  double sum = 0;
  bool all_int = true;
  as xformer;
  for (int i = 0; i < num_args; i++) {
    IRNode* arg = va_arg(args, IRNode*);
    if (arg->type == ASTNodeType::Decimal) {
      all_int = false;
      xformer.integer = arg->value;
      sum += xformer.decimal;
    } else if (arg->type != ASTNodeType::Integer) {
      std::cerr << "Add received an incompatible operand type";
      exit(1);
    } else {
      sum += arg->value;
    }
  }

  if (not all_int) {
    auto ret = new IRNode();
    ret->type = ASTNodeType::Decimal;
    xformer.decimal = sum;
    ret->value = xformer.integer;
    return ret;
  }
  auto ret = new IRNode();
  ret->type = ASTNodeType::Integer;
  ret->value = (uint64_t)sum;
  return ret;
}

void printNode(IRNode* node) {
  as xformer;
  xformer.integer = node->value;
  if (node->type == ASTNodeType::Integer)
    std::cout << xformer.integer << std::endl;
  else if (node->type == ASTNodeType::Decimal)
    std::cout << xformer.decimal << std::endl;
  else
    std::cerr << "Unsupported" << std::endl;
}
