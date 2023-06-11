#include "trt.h"

#include <cstdarg>
#include <iostream>
#include <list>
#include <map>

static std::map<int64_t, std::list<IRNode*>> gEnv;

typedef union _as {
  int64_t integer;
  double decimal;
} as;

enum ASTNodeType { Bool = 0, Integer, Decimal, Symbol, List, Lambda, String };
IRNode* define(IRNode* symbol, IRNode* value) {
  if (symbol->type != ASTNodeType::Symbol)
    throw std::runtime_error("Non-symbol can't be defined");
  if (gEnv.find(symbol->value) != gEnv.end() && gEnv[symbol->value].size()) {
    gEnv[symbol->value].pop_front();
    gEnv[symbol->value].push_front(value);
  } else {
    gEnv[symbol->value].push_front(value);
  }
  return value;
}

IRNode* retrieve(IRNode* symbol) {
  if (symbol->type != ASTNodeType::Symbol) {
    throw std::runtime_error("Can't retrieve value of non-symbol");
  }
  if (gEnv.find(symbol->value) != gEnv.end() && gEnv[symbol->value].size()) {
    return gEnv[symbol->value].front();
  }
  throw std::runtime_error("Undefined symbol");
}

IRNode* is_equal(IRNode* oprnd1, IRNode* oprnd2) {
  auto ret = new IRNode();
  ret->type = ASTNodeType::Bool;
  if (oprnd1->type == oprnd2->type && oprnd1->value == oprnd2->value)
    ret->value = 1;
  else
    ret->value = 0;
  return ret;
}

IRNode* is_type(IRNode* oprnd1, uint32_t type) {
  auto ret = new IRNode();
  ret->type = ASTNodeType::Bool;
  if (oprnd1->type == type)
    ret->value = 1;
  else
    ret->value = 0;
  return ret;
}

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

  if (num_args < 2) {
    std::runtime_error("Too few arguments..");
  }

  int i = 0;
  if (op == '-' || op == '/') {
    i++;
    IRNode* arg = va_arg(args, IRNode*);
    if (arg->type == ASTNodeType::Decimal) {
      all_int = false;
      xformer.integer = arg->value;
      acc = operation(acc, xformer.decimal, '+');
    } else if (arg->type != ASTNodeType::Integer) {
      std::cerr << "Add received an incompatible operand type";
      exit(1);
    } else {
      acc = operation(acc, arg->value, '+');
    }
  }

  for (; i < num_args; i++) {
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
  else if (node->type == ASTNodeType::Bool)
    std::cout << (xformer.decimal ? "#true" : "#false") << std::endl;
  else
    std::cerr << "Unsupported" << std::endl;
}
