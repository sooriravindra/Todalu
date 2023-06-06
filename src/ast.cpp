#include "ast.h"

#include <iostream>
#include <sstream>
#include <string>

#include "common.h"
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

std::list<ASTNode*> create_ast(std::list<std::string>& tokens,
                               bool closing_paren_allow) {
  std::list<ASTNode*> ret;
  if (tokens.empty())
    throw TodaluException("Unexpected EOF while reading input");

  while (tokens.size()) {
    std::string t = tokens.front();
    tokens.pop_front();
    ASTNode* node;
    if (t == "(") {
      node = new ListNode(create_ast(tokens, true));
    } else if (t == ")") {
      if (not closing_paren_allow) throw TodaluException("Unexpected ')'");
      return ret;
    } else if (t.starts_with("\"")) {
      std::string str = t;
      while (!t.ends_with("\"")) {
        if (!tokens.size()) throw TodaluException("Unmatched '\"'");
        t = tokens.front();
        tokens.pop_front();
        str += " ";
        str += t;
      }
      node = new StringNode(str.substr(1, str.length() - 2));
    } else if (is_int(t)) {
      node = new IntegerNode(std::stoi(t));
    } else if (is_float(t)) {
      node = new DecimalNode(get_float(t));
    } else {
      // Handle bool in granthalaya, symbol for now
      node = new SymbolNode(t);
    }
    ret.push_back(node);
  }
  if (closing_paren_allow) throw TodaluException("Unmatched '('");
  return ret;
}
