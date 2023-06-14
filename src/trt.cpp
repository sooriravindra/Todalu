#include "trt.h"

#include <cstdarg>
#include <iostream>
#include <list>
#include <map>

static std::map<int64_t, std::list<IRNode*>> gEnv;

typedef union _as {
  int64_t integer;
  double decimal;
  std::list<IRNode*>* list;
  char* str;
} as;

enum ASTNodeType { Bool = 0, Integer, Decimal, Symbol, List, Lambda, String };
IRNode* define(IRNode* symbol, IRNode* value, bool shouldPop) {
  if (symbol->type != ASTNodeType::Symbol)
    throw std::runtime_error("Non-symbol can't be defined");
  if (gEnv.find(symbol->value) != gEnv.end() && gEnv[symbol->value].size()) {
    if (shouldPop) gEnv[symbol->value].pop_front();
    gEnv[symbol->value].push_front(value);
  } else {
    gEnv[symbol->value].push_front(value);
  }
  return value;
}

bool evaluateCondition(IRNode* node) {
  if (node->type == ASTNodeType::List) {
    std::list<IRNode*>* plist = (std::list<IRNode*>*)node->value;
    return (plist->size() != 0);
  }
  return (node->value != 0);
}

IRNode* undefine(IRNode* symbol) {
  if (symbol->type != ASTNodeType::Symbol)
    throw std::runtime_error("Non-symbol can't be undefined");
  if (gEnv.find(symbol->value) != gEnv.end() && gEnv[symbol->value].size()) {
    gEnv[symbol->value].pop_front();
  } else {
    throw std::runtime_error("Can't undefine a symbol that is not defined");
  }
  return symbol;
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

IRNode* is_greater(IRNode* oprnd1, IRNode* oprnd2) {
  if ((oprnd1->type != ASTNodeType::Integer &&
       oprnd1->type != ASTNodeType::Decimal) ||
      (oprnd2->type != ASTNodeType::Integer &&
       oprnd2->type != ASTNodeType::Decimal))
    throw std::runtime_error("Can't compare operands of non-numeric types");
  auto ret = new IRNode();
  ret->type = ASTNodeType::Bool;
  if (oprnd1->type == oprnd2->type) {
    if (oprnd1->value > oprnd2->value)
      ret->value = 1;
    else
      ret->value = 0;
  } else {
    as xformer;
    xformer.integer = oprnd1->value;
    double x = oprnd1->type == ASTNodeType::Integer ? (double)xformer.integer
                                                    : xformer.decimal;
    xformer.integer = oprnd2->value;
    double y = oprnd2->type == ASTNodeType::Integer ? (double)xformer.integer
                                                    : xformer.decimal;
    if (x > y)
      ret->value = 1;
    else
      ret->value = 0;
  }
  return ret;
}

void quit(IRNode* node) {
  if (node->type != ASTNodeType::Integer)
    throw std::runtime_error("Got non-integer exit code");
  exit(node->value);
}

IRNode* allocNode() { return new IRNode(); }

char* allocList() { return (char*)new std::list<IRNode*>(); }

void listPushBack(char* list, IRNode* node) {
  ((std::list<IRNode*>*)list)->push_back(node);
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
  va_end(args);

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

IRNode* deepCopy(IRNode* node) {
  auto ret = allocNode();
  switch (node->type) {
    case ASTNodeType::Bool:
    case ASTNodeType::Decimal:
    case ASTNodeType::Integer:
      ret->value = node->value;
      ret->type = node->type;
      return ret;
    case ASTNodeType::Lambda:
    case ASTNodeType::List: {
      ret->type = node->type;
      auto list = ((std::list<IRNode*>*)node->value);
      auto retlist = ((std::list<IRNode*>*)allocList());
      for (auto it = list->begin(); it != list->end(); it++) {
        retlist->push_back(deepCopy(*it));
      }
      ret->value = (int64_t)retlist;
      return ret;
    }
    case ASTNodeType::Symbol:
    case ASTNodeType::String:;
  }
  throw std::runtime_error("Not implemented");
}

IRNode* throwException() {
  throw std::runtime_error("There was an exception");
  return nullptr;
}

IRNode* executeLambda(IRNode* lambda, int argc, ...) {
  va_list args;
  va_start(args, argc);
  if (lambda->type != ASTNodeType::Lambda)
    throw std::runtime_error("List head not a lambda");
  auto pls = (LambdaStruct*)lambda->value;
  if (pls->argc != argc) throw std::runtime_error("Lambda argument mismatch");
  // bind args
  for (auto it = pls->arglist->begin(); it != pls->arglist->end(); it++) {
    define(*it, va_arg(args, IRNode*), false);
  }
  va_end(args);
  // call lambda body
  auto ret = pls->fun();
  // unbind args
  for (auto it = pls->arglist->begin(); it != pls->arglist->end(); it++) {
    undefine(*it);
  }
  return ret;
}

IRNode* createLambda(void* fun, int argc, ...) {
  va_list args;
  va_start(args, argc);
  auto node = allocNode();
  node->type = ASTNodeType::Lambda;
  auto pls = new LambdaStruct();
  pls->fun = (IRNode * (*)()) fun;
  pls->argc = argc;
  pls->arglist = new std::list<IRNode*>();
  for (int i = 0; i < argc; i++) {
    auto sym = va_arg(args, IRNode*);
    pls->arglist->push_back(sym);
  }
  va_end(args);
  node->value = (int64_t)pls;
  return node;
}

IRNode* car(IRNode* listnode) {
  if (listnode->type != ASTNodeType::List)
    throw std::runtime_error("car expects a list node");
  return deepCopy(((std::list<IRNode*>*)listnode->value)->front());
}

IRNode* cdr(IRNode* listnode) {
  if (listnode->type != ASTNodeType::List)
    throw std::runtime_error("car expects a list node");
  auto node = deepCopy(listnode);
  auto list = ((std::list<IRNode*>*)node->value);
  delete list->front();
  list->pop_front();
  return node;
}

IRNode* cons(IRNode* addNode, IRNode* listnode) {
  if (listnode->type != ASTNodeType::List)
    throw std::runtime_error("car expects a list node");
  auto node = deepCopy(listnode);
  auto list = ((std::list<IRNode*>*)node->value);
  list->push_front(addNode);
  return node;
}

void printNode(IRNode* node, char endchar) {
  std::string end = "";
  if (endchar) end += endchar;
  as xformer;
  xformer.integer = node->value;
  switch (node->type) {
    case ASTNodeType::Integer:
      std::cout << xformer.integer << end;
      break;
    case ASTNodeType::Decimal:
      std::cout << xformer.decimal << end;
      break;
    case ASTNodeType::Bool:
      std::cout << (xformer.decimal ? "#true" : "#false") << end;
      break;
    case ASTNodeType::List: {
      auto list = xformer.list;
      std::cout << "( ";
      for (auto it = list->begin(); it != list->end(); it++) {
        printNode(*it, ' ');
      }
      std::cout << ")" << end;
      break;
    }
    case ASTNodeType::String:
      std::cout << xformer.str << end;
      break;
    case ASTNodeType::Lambda:
      std::cout << "<lambda=" << xformer.decimal << ">" << end;
    default:
      std::cerr << "Unsupported" << std::endl;
  }
}
