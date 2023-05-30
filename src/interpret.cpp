#include "interpret.h"

#include <iostream>
#include <list>
#include <regex>
#include <string>

enum class ASTNodeType { Integer, Decimal, Symbol, List, Lambda };

typedef struct _ASTNode {
  ASTNodeType type;
  std::string repr;
  // TODO Make this a union or a super class and dervied classes
  int integer;
  float decimal;
  std::list<struct _ASTNode> list;
} ASTNode;

std::map<std::string, std::list<ASTNode>> gEnv;

std::list<std::string> tokenizer(std::string& str) {
  str = std::regex_replace(str, std::regex("\\("), " ( ");
  str = std::regex_replace(str, std::regex("\\)"), " ) ");
  std::list<std::string> tokens;
  std::stringstream ss(str);
  std::string token;
  while (ss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}

bool is_int(const std::string& str) {
  try {
    size_t pos;
    std::stoi(str, &pos);
    return pos == str.length();
  } catch (...) {
    return false;
  }
}

bool is_float(const std::string& input) {
  std::istringstream iss(input);
  float value;
  iss >> std::noskipws >> value;
  return iss.eof() && !iss.fail();
}

float get_float(const std::string& input) {
  std::istringstream iss(input);
  float value;
  iss >> std::noskipws >> value;
  return value;
}

std::string get_repr(std::list<ASTNode>& ast) {
  std::string res = "";
  auto it = ast.begin();
  while (it != ast.end()) {
    switch (it->type) {
    case ASTNodeType::Integer:
    case ASTNodeType::Decimal:
    case ASTNodeType::Symbol:
      res += it->repr;
      res += " ";
      break;
    case ASTNodeType::List:
      res += "( ";
      res += get_repr(it->list);
      res += ") ";
      break;
    case ASTNodeType::Lambda:
      res += "$lambda$ ";
      break;
    }
    it++;
  }
  return res;
}

std::list<ASTNode> create_ast(std::list<std::string>& tokens,
                              bool closing_paren_allow = false) {
  std::list<ASTNode> ret;
  if (tokens.empty()) throw ParseError("Unexpected EOF while reading input");

  while (tokens.size()) {
    std::string t = tokens.front();
    tokens.pop_front();
    if (t == "(") {
      auto node = new ASTNode();
      node->type = ASTNodeType::List;
      node->list = create_ast(tokens, true);
      node->repr = get_repr(node->list);
      ret.push_back(*node);
    } else if (t == ")") {
      if (not closing_paren_allow) throw ParseError("Unexpected ')'");
      return ret;
    } else if (is_int(t)) {
      auto node = new ASTNode();
      node->type = ASTNodeType::Integer;
      node->integer = std::stoi(t);
      node->repr = t;
      ret.push_back(*node);
    } else if (is_float(t)) {
      auto node = new ASTNode();
      node->type = ASTNodeType::Decimal;
      node->decimal = get_float(t);
      node->repr = t;
      ret.push_back(*node);
    } else {
      auto node = new ASTNode();
      node->type = ASTNodeType::Symbol;
      node->repr = t;
      ret.push_back(*node);
    }
  }
  return ret;
}

void eval_tree(std::list<ASTNode>::iterator node);
void operate_on_node(std::list<ASTNode>::iterator node, char op, float& acc,
                     bool& is_all_int) {
  auto operation = [](float a, float b, char op) {
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
  eval_tree(node);
  switch (node->type) {
  case ASTNodeType::Integer:
    acc = operation(acc, node->integer, op);
    break;
  case ASTNodeType::Decimal:
    acc = operation(acc, node->decimal, op);
    is_all_int = false;
    break;
  default:
    throw ParseError("Unsuitable operands to +");
  }
}

void eval_tree(std::list<ASTNode>::iterator node) {
  // TODO Evaluaute AST here
  if (node->type == ASTNodeType::List) {
    if (node->list.front().type == ASTNodeType::Symbol) {
      auto fun = node->list.front().repr;
      if (fun.length() == 1 &&
          (fun[0] == '+' || fun[0] == '-' || fun[0] == '*' || fun[0] == '/')) {
        auto it = node->list.begin();

        if (node->list.size() < 3) throw ParseError("Needs atleast 2 operands");

        it++;

        float acc = (fun[0] == '*') ? 1 : 0;
        bool is_all_int = true;

        if (fun[0] == '-' || fun[0] == '/') {
          operate_on_node(it, '+', acc, is_all_int);
          it++;
        }

        while (it != node->list.end()) {
          operate_on_node(it, fun[0], acc, is_all_int);
          it++;
        }

        if (is_all_int) {
          node->type = ASTNodeType::Integer;
          node->integer = (int)(acc);
          node->repr = std::to_string((int)acc);
        } else {
          node->type = ASTNodeType::Decimal;
          node->decimal = acc;
          node->repr = std::to_string(acc);
        }
        // TODO clear list
        return;
      }
      if (fun == "quote") {
        node->list.pop_front();
        if (node->list.size() != 1)
          throw ParseError("quote expects one argument");
        node->type = node->list.front().type;
        node->integer = node->list.front().integer;
        node->decimal = node->list.front().decimal;
        node->repr = node->list.front().repr;
        auto x = node->list.front().list;
        // TODO free memory here
        node->list = x;
        return;
      }
      if (fun == "car") {
        node->list.pop_front();
        if (node->list.size() != 1)
          throw ParseError("car expects one argument");
        eval_tree(node->list.begin());
        if (node->list.front().type != ASTNodeType::List)
          throw ParseError("car expects argument of type list");
        node->type = node->list.front().list.front().type;
        node->integer = node->list.front().list.front().integer;
        node->decimal = node->list.front().list.front().decimal;
        node->repr = node->list.front().list.front().repr;
        auto x = node->list.front().list.front().list;
        // TODO free memory here
        node->list = x;
        return;
      }
      if (fun == "cdr") {
        node->list.pop_front();
        if (node->list.size() != 1)
          throw ParseError("car expects one argument");
        eval_tree(node->list.begin());
        if (node->list.front().type != ASTNodeType::List)
          throw ParseError("car expects argument of type list");
        node->list.front().list.pop_front();
        node->type = ASTNodeType::List;
        node->integer = 0;
        node->decimal = 0;
        node->repr = "";
        auto x = node->list.front().list;
        // TODO free memory here
        node->list = x;
        return;
      }
      if (fun == "lambda") {
        if (node->list.size() != 3) throw ParseError("lambda syntax incorrect");
        node->type = ASTNodeType::Lambda;
        node->integer = 0;
        node->decimal = 0;
        node->repr = "$lambda$";
        node->list.pop_front();

        if (node->list.front().type == ASTNodeType::List) {
          auto l = node->list.front().list.begin();
          while (l != node->list.front().list.end()) {
            if (l->type != ASTNodeType::Symbol)
              throw ParseError("lambda argument list has non-symbol");
            l++;
          }
        } else if (node->list.front().type != ASTNodeType::Symbol) {
          throw ParseError("lambda argument has non-symbol");
        }
        return;
      }
      if (fun == "def") {
        node->list.pop_front();
        if (node->list.size() != 2)
          throw ParseError("def expects two arguments");
        if (node->list.front().type != ASTNodeType::Symbol)
          throw ParseError("def expects first argument to be symbol");
        auto key = node->list.begin();
        auto value = std::next(key);
        eval_tree(value);
        gEnv[key->repr].push_front(*value);
        node->type = value->type;
        node->integer = value->integer;
        node->decimal = value->decimal;
        node->repr = value->repr;
        auto l = value->list;
        node->list = l;
        return;
      }
    }
    // Try this as lambda
    eval_tree(node->list.begin());
    if (node->list.front().type != ASTNodeType::Lambda)
      throw ParseError(std::string("Invalid function : ") +
                       node->list.front().repr);

  } else if (node->type == ASTNodeType::Symbol) {
    auto x = gEnv.find(node->repr);
    if (x != gEnv.end()) {
      auto y = x->second.front();
      node->type = y.type;
      node->decimal = y.decimal;
      node->integer = y.integer;
      node->list = y.list;
      node->repr = y.repr;
    } else {
      throw ParseError(std::string("Undefined symbol : ") + node->repr);
    }
  }
}

std::string interpret_line_internal(std::string str) {
  auto tokens = tokenizer(str);
  auto ast = create_ast(tokens);

  if (ast.size() == 0) return "";

  if (ast.size() != 1)
    throw ParseError("Contains more than one node at the base");

  eval_tree(ast.begin());

  return get_repr(ast);
}

std::string interpret_line(std::string str) {
  return interpret_line_internal(str);
}
