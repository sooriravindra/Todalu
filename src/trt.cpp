#include "trt.h"

#include <cstdarg>
#include <iostream>

typedef union _as {
  int64_t integer;
  double decimal;
} as;

enum ASTNodeType { Bool = 0, Integer, Decimal, Symbol, List, Lambda, String };
IRNode* arithmetic(char op, uint32_t num_args, ...) {
  va_list args;
  va_start(args, num_args);
  double acc = op == '*' ? 1 : 0;
  bool all_int = true;
  as xformer;
  auto operation = [](double a, double b, char op) {
    switch (op) {
      case '+':
        return a + b;
      case '-':
        return a - b;
      case '*':
        return a * b;
      case '/':
        return a / b;
      default:
        throw std::runtime_error("Invalid operation");
    }
  };

  if (op == '-' || op == '/') {
    throw std::runtime_error("Not implemented yet");
  }

  for (int i = 0; i < num_args; i++) {
    IRNode* arg = va_arg(args, IRNode*);
    if (arg->type == ASTNodeType::Decimal) {
      all_int = false;
      xformer.integer = arg->value;
      acc = operation(acc, xformer.decimal, op);
    } else if (arg->type != ASTNodeType::Integer) {
      std::cerr << "Add received an incompatible operand type";
      exit(1);
    } else {
      acc = operation(acc, arg->value, op);
    }
  }

  if (not all_int) {
    auto ret = new IRNode();
    ret->type = ASTNodeType::Decimal;
    xformer.decimal = acc;
    ret->value = xformer.integer;
    return ret;
  }
  auto ret = new IRNode();
  ret->type = ASTNodeType::Integer;
  ret->value = (uint64_t)acc;
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
