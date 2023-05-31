#include <list>
#include <string>

enum class ASTNodeType { Bool, Integer, Decimal, Symbol, List, Lambda };

class ASTNode {
public:
  virtual std::string getRepr() = 0;
  virtual bool getBool() = 0;
  virtual ASTNodeType type() = 0;
};

class BoolNode : public ASTNode {
public:
  BoolNode(bool v) : value(v) {}
  ASTNodeType type() { return ASTNodeType::Bool; }
  bool getBool() {
    return value;
  }
  std::string getRepr() {
    if (value)
      return "t";
    return "nil";
  }
  bool value = false;
};


class IntegerNode : public ASTNode {
public:
  IntegerNode(int v) : value(v) {}
  ASTNodeType type() { return ASTNodeType::Integer; }
  bool getBool() {
    return (value != 0);
  }
  std::string getRepr() {
    return std::to_string(value);
  }
  int value = 0;
};

class DecimalNode : public ASTNode {
public:
  DecimalNode(float v) : value(v) {}
  ASTNodeType type() { return ASTNodeType::Decimal; }
  bool getBool() {
    return (value != 0);
  }
  std::string getRepr() {
    return std::to_string(value);
  }
  float value = 0;
};

class SymbolNode : public ASTNode {
public:
  SymbolNode(std::string v) : symbol(v) {}
  ASTNodeType type() { return ASTNodeType::Symbol; }
  bool getBool() {
    return true;
  }
  std::string getRepr() {
    return symbol;
  }
  std::string symbol;
};

class LambdaNode : public ASTNode {
public:
  LambdaNode(ASTNode* x, ASTNode* y) : arglist(x), body(y) {}
  ASTNodeType type() { return ASTNodeType::Lambda; }
  bool getBool() {
    return true;
  }
  std::string getRepr() {
    return std::string("<lambda=") + std::to_string((uint64_t)this) + ">";
  }
  ASTNode* arglist = nullptr;
  ASTNode* body = nullptr;
};

class ListNode : public ASTNode {
public:
  ListNode(std::list<ASTNode*> l) : list(l) {}
  ASTNodeType type() { return ASTNodeType::List; }
  bool getBool() {
    return (list.size() != 0);
  }
  std::string getRepr() {
    std::string lr = "( ";
    for (auto it = list.begin(); it != list.end(); it++) {
      lr += (*it)->getRepr();
      lr += " ";
    }
    lr += ")";
    return lr;
  }
  std::list<ASTNode*> list;
};
