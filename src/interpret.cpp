#include "interpret.h"

#include <iostream>
#include <list>
#include <regex>
#include <string>

#include "ast.h"

std::map<std::string, std::list<ASTNode*>> gEnv;

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

std::list<ASTNode*> create_ast(std::list<std::string>& tokens,
                               bool closing_paren_allow = false) {
  std::list<ASTNode*> ret;
  if (tokens.empty()) throw ParseError("Unexpected EOF while reading input");

  while (tokens.size()) {
    std::string t = tokens.front();
    tokens.pop_front();
    ASTNode* node;
    if (t == "(") {
      node = new ListNode(create_ast(tokens, true));
    } else if (t == ")") {
      if (not closing_paren_allow) throw ParseError("Unexpected ')'");
      return ret;
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
  if (closing_paren_allow) throw ParseError("Unmatched '('");
  return ret;
}

ASTNode* eval_tree(ASTNode* node);
void operate_on_node(ASTNode* node, char op, float& acc, bool& is_all_int) {
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
  auto operand = eval_tree(node);
  switch (operand->type()) {
    case ASTNodeType::Integer:
      acc = operation(acc, dynamic_cast<IntegerNode*>(operand)->value, op);
      break;
    case ASTNodeType::Decimal:
      acc = operation(acc, dynamic_cast<DecimalNode*>(operand)->value, op);
      is_all_int = false;
      break;
    default:
      throw ParseError("Unsuitable operands to operator");
  }
}

void bind_arguments(LambdaNode* lambda, ListNode* listnode) {
  if (lambda->arglist->type() == ASTNodeType::Symbol) {
    auto arg = eval_tree(listnode->list.back());  // there is only 1 arg
    gEnv[lambda->arglist->getRepr()].push_front(arg);
  } else if (lambda->arglist->type() == ASTNodeType::List) {
    auto it = dynamic_cast<ListNode*>(lambda->arglist)->list.begin();
    auto itend = dynamic_cast<ListNode*>(lambda->arglist)->list.end();
    auto itarg =
        std::next(listnode->list.begin());  // 1st node in list is lambda
    while (it != itend) {
      gEnv[(*it)->getRepr()].push_front(eval_tree(*itarg));
      it++;
      itarg++;
    }
  }
}

void unbind_arguments(LambdaNode* lambda) {
  if (lambda->arglist->type() == ASTNodeType::Symbol) {
    gEnv[lambda->arglist->getRepr()].pop_front();
    if (gEnv[lambda->arglist->getRepr()].size() == 0)
      gEnv.erase(lambda->arglist->getRepr());
  } else if (lambda->arglist->type() == ASTNodeType::List) {
    auto it = dynamic_cast<ListNode*>(lambda->arglist)->list.begin();
    auto itend = dynamic_cast<ListNode*>(lambda->arglist)->list.end();
    while (it != itend) {
      gEnv[(*it)->getRepr()].pop_front();
      if (gEnv[(*it)->getRepr()].size() == 0) gEnv.erase((*it)->getRepr());
      it++;
    }
  }
}

ASTNode* eval_tree(ASTNode* node) {
  if (node->type() == ASTNodeType::List) {
    auto listnode = dynamic_cast<ListNode*>(node);
    if (listnode->list.front()->type() == ASTNodeType::Symbol) {
      auto fun = dynamic_cast<SymbolNode*>(listnode->list.front())->symbol;

      if (fun == "+" || fun == "-" || fun == "*" || fun == "/") {
        auto it = listnode->list.begin();
        it++;
        if (listnode->list.size() < 3)
          throw ParseError("Needs atleast 2 operands");
        float acc = (fun[0] == '*') ? 1 : 0;
        bool is_all_int = true;
        if (fun == "-" || fun == "/") {
          operate_on_node(*it, '+', acc, is_all_int);
          it++;
        }
        while (it != listnode->list.end()) {
          operate_on_node(*it, fun[0], acc, is_all_int);
          it++;
        }
        if (is_all_int) return new IntegerNode(acc);
        return new DecimalNode(acc);
      }

      if (fun == "eq?") {
        if (listnode->list.size() != 3)
          throw ParseError("eq? expects two arguments");

        auto oprnd1 = eval_tree(*(std::next(listnode->list.begin())));
        auto oprnd2 = eval_tree(listnode->list.back());

        bool res = false;
        if (oprnd1->type() == oprnd2->type() &&
            ((oprnd1->type() == ASTNodeType::Integer &&
              (dynamic_cast<IntegerNode*>(oprnd1)->value ==
               dynamic_cast<IntegerNode*>(oprnd2)->value)) ||
             (oprnd1->type() == ASTNodeType::Decimal &&
              (dynamic_cast<DecimalNode*>(oprnd1)->value ==
               dynamic_cast<DecimalNode*>(oprnd2)->value))))
          res = true;

        return new BoolNode(res);
      }

      if (fun == "list?") {
        if (listnode->list.size() != 2)
          throw ParseError("list? expects one argument");
        auto oprnd = eval_tree(listnode->list.back());
        return new BoolNode(oprnd->type() == ASTNodeType::List);
      }

      if (fun == "int?") {
        if (listnode->list.size() != 2)
          throw ParseError("int? expects one argument");
        auto oprnd = eval_tree(listnode->list.back());
        return new BoolNode(oprnd->type() == ASTNodeType::Integer);
      }

      if (fun == "bool?") {
        if (listnode->list.size() != 2)
          throw ParseError("bool? expects one argument");
        auto oprnd = eval_tree(listnode->list.back());
        return new BoolNode(oprnd->type() == ASTNodeType::Bool);
      }

      if (fun == "dec?") {
        if (listnode->list.size() != 2)
          throw ParseError("dec? expects one argument");
        auto oprnd = eval_tree(listnode->list.back());
        return new BoolNode(oprnd->type() == ASTNodeType::Decimal);
      }

      if (fun == ">") {
        if (listnode->list.size() != 3)
          throw ParseError("> expects two arguments");

        auto oprnd1 = eval_tree(*(std::next(listnode->list.begin())));
        auto oprnd2 = eval_tree(listnode->list.back());

        bool res = false;
        if (oprnd1->type() == oprnd2->type() &&
            ((oprnd1->type() == ASTNodeType::Integer &&
              (dynamic_cast<IntegerNode*>(oprnd1)->value >
               dynamic_cast<IntegerNode*>(oprnd2)->value)) ||
             (oprnd1->type() == ASTNodeType::Decimal &&
              (dynamic_cast<DecimalNode*>(oprnd1)->value >
               dynamic_cast<DecimalNode*>(oprnd2)->value))))
          res = true;

        return new BoolNode(res);
      }

      if (fun == "progn") {
        if (listnode->list.size() < 2)
          throw ParseError("progn expects atleast one element");
        auto it = std::next(listnode->list.begin());
        ASTNode* ret = nullptr;
        while (it != listnode->list.end()) {
          ret = eval_tree(*it);
          it++;
        }
        return ret;
      }

      if (fun == "quote") {
        if (listnode->list.size() != 2)
          throw ParseError("quote expects one argument");
        return listnode->list.back();
      }

      if (fun == "eval") {
        if (listnode->list.size() != 2)
          throw ParseError("eval expects one argument");
        return eval_tree(
            eval_tree(listnode->list.back()));  // Once to get the operand,
                                                // second actually evaluates
      }

      if (fun == "car") {
        if (listnode->list.size() != 2)
          throw ParseError("car expects one argument");
        auto oprnd = eval_tree(listnode->list.back());
        if (oprnd->type() != ASTNodeType::List)
          throw ParseError("car expects argument of type list");
        // TODO free memory here?
        return dynamic_cast<ListNode*>(oprnd)->list.front();
      }

      if (fun == "cdr") {
        if (listnode->list.size() != 2)
          throw ParseError("cdr expects one argument");
        auto oprnd = eval_tree(listnode->list.back());
        if (oprnd->type() != ASTNodeType::List)
          throw ParseError("cdr expects argument of type list");

        auto ret = dynamic_cast<ListNode*>(oprnd)->list;
        ret.pop_front();
        return new ListNode(ret);
      }

      if (fun == "cons") {
        if (listnode->list.size() != 3)
          throw ParseError("cons expects two arguments");
        auto oprnd1 = eval_tree(*(std::next(listnode->list.begin())));
        auto oprnd2 = eval_tree(listnode->list.back());
        if (oprnd2->type() != ASTNodeType::List)
          throw ParseError("cons expects second argument of type list");

        auto ret = dynamic_cast<ListNode*>(oprnd2)->list;
        ret.push_front(oprnd1);
        return new ListNode(ret);
      }

      if (fun == "lambda") {
        if (listnode->list.size() != 3)
          throw ParseError("lambda syntax incorrect");
        auto arglist = *std::next(listnode->list.begin());
        auto body = listnode->list.back();

        if (arglist->type() == ASTNodeType::List) {
          auto l = dynamic_cast<ListNode*>(arglist)->list.begin();
          while (l != dynamic_cast<ListNode*>(arglist)->list.end()) {
            if ((*l)->type() != ASTNodeType::Symbol)
              throw ParseError("lambda argument list has non-symbol");
            l++;
          }
        } else if (arglist->type() != ASTNodeType::Symbol) {
          throw ParseError("lambda argument has non-symbol");
        }

        auto lambdanode = new LambdaNode(arglist, body);
        return lambdanode;
      }

      if (fun == "def") {
        if (listnode->list.size() != 3)
          throw ParseError("def expects two arguments");
        auto sym = *(std::next(listnode->list.begin()));
        if (sym->type() != ASTNodeType::Symbol)
          throw ParseError(
              std::string("def expects first argument to be symbol. Found ") +
              sym->getRepr());
        auto value = eval_tree(listnode->list.back());
        gEnv[sym->getRepr()].push_front(value);
        return value;
      }

      if (fun == "if") {
        if (listnode->list.size() != 4)
          throw ParseError("if expects cond,body and else parts");
        auto predicate = eval_tree(*(std::next(listnode->list.begin())));
        auto body = *(std::next(std::next(listnode->list.begin())));
        if (predicate->getBool() == false) {
          body = listnode->list.back();
        }
        return eval_tree(body);
      }
    }
    // Treat this as lambda and try to execute
    auto lambda_candidate = eval_tree(listnode->list.front());
    if (lambda_candidate->type() != ASTNodeType::Lambda)
      throw ParseError(std::string("Invalid function : ") +
                       lambda_candidate->getRepr());

    auto lambda = dynamic_cast<LambdaNode*>(lambda_candidate);

    if ((lambda->arglist->type() == ASTNodeType::Symbol &&
         listnode->list.size() != 2) ||
        (lambda->arglist->type() == ASTNodeType::List &&
         listnode->list.size() !=
             dynamic_cast<ListNode*>(lambda->arglist)->list.size() + 1)) {
      throw ParseError("lambda argument count mismatch");
    }

    bind_arguments(lambda, listnode);
    ASTNode* result = nullptr;
    try {
      result = eval_tree(lambda->body);
    } catch (...) {
      unbind_arguments(lambda);
      throw;
    }
    unbind_arguments(lambda);
    return result;

  } else if (node->type() == ASTNodeType::Symbol) {
    if (node->getRepr() == "#fail") {
      throw ParseError("Exception thrown!");
    }
    auto pair = gEnv.find(node->getRepr());
    if (pair != gEnv.end()) {
      return pair->second.front();  // second is list of possible values
    } else {
      throw ParseError(std::string("Undefined symbol : ") + node->getRepr());
    }
  }
  return node;
}

bool is_comment(std::string& line) {
  for (auto c : line) {
    if (std::isspace(c)) continue;
    if (c == '#') return true;
    return false;
  }
  return true;
}

std::string interpret_line(std::string str) {
  if (is_comment(str)) return "";
  auto tokens = tokenizer(str);
  auto ast = create_ast(tokens);

  if (ast.size() == 0) return "";

  if (ast.size() != 1)
    throw ParseError("Contains more than one node at the base");

  return eval_tree(ast.front())->getRepr() + "\n";
}
