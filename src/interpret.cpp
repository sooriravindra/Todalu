#include "interpret.h"

#include <iostream>
#include <list>
#include <regex>
#include <string>

enum class ASTNodeType { Integer, Decimal, Symbol, List };

typedef struct _ASTNode {
  ASTNodeType type;
  std::string repr;
  // TODO Make this a union or a super class and dervied classes
  int integer;
  float decimal;
  std::list<struct _ASTNode> list;
} ASTNode;

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

void eval_tree(std::list<ASTNode>::iterator node) {
  // TODO Evaluaute AST here
  if (node->type == ASTNodeType::List) {
    if (node->list.front().type == ASTNodeType::Symbol) {
      auto fun = node->list.front().repr;
      if (fun == "+") {
        auto it = node->list.begin();
        it++;
        float sum = 0;
        bool is_all_int = true;
        while (it != node->list.end()) {
          eval_tree(it);
          switch (it->type) {
            case ASTNodeType::Integer:
              sum += it->integer;
              break;
            case ASTNodeType::Decimal:
              sum += it->decimal;
              is_all_int = false;
              break;
            default:
              throw ParseError("Unsuitable operands to +");
          }
          it++;
        }
        if (is_all_int) {
          node->type = ASTNodeType::Integer;
          node->integer = (int)(sum);
          node->repr = std::to_string((int)sum);
        } else {
          node->type = ASTNodeType::Decimal;
          node->decimal = sum;
          node->repr = std::to_string(sum);
        }
      }
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
  auto res = interpret_line_internal(str);
  return res + "\n";
}
