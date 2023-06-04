#include <list>
#include <string>

enum class ASTNodeType { Bool, Integer, Decimal, Symbol, List, Lambda, String };

class ASTNode {
 public:
  virtual std::string getRepr() = 0;
  virtual bool getBool() = 0;
  virtual ASTNodeType type() = 0;
  virtual ASTNode* deepCopy() { return nullptr; };
  virtual ~ASTNode() {}
};

class BoolNode : public ASTNode {
 public:
  BoolNode(bool v) : value(v) {}
  ASTNodeType type() { return ASTNodeType::Bool; }
  bool getBool() { return value; }
  std::string getRepr() {
    if (value) return "#true";
    return "#false";
  }
  ASTNode* deepCopy() { return new BoolNode(value); }
  bool value = false;
};

class IntegerNode : public ASTNode {
 public:
  IntegerNode(int v) : value(v) {}
  ASTNodeType type() { return ASTNodeType::Integer; }
  bool getBool() { return (value != 0); }
  std::string getRepr() { return std::to_string(value); }
  ASTNode* deepCopy() { return new IntegerNode(value); }
  int value = 0;
};

class DecimalNode : public ASTNode {
 public:
  DecimalNode(float v) : value(v) {}
  ASTNodeType type() { return ASTNodeType::Decimal; }
  bool getBool() { return (value != 0); }
  std::string getRepr() { return std::to_string(value); }
  ASTNode* deepCopy() { return new DecimalNode(value); }
  float value = 0;
};

class StringNode : public ASTNode {
 public:
  StringNode(std::string v) : value(v) {}
  ASTNodeType type() { return ASTNodeType::String; }
  bool getBool() { return !value.empty(); }
  std::string getRepr() {
    std::string repr = "\"";
    repr += value;
    repr += "\"";
    return repr;
  }
  ASTNode* deepCopy() { return new StringNode(value); }
  std::string value = "";
};

class SymbolNode : public ASTNode {
 public:
  SymbolNode(std::string v) : symbol(v) {}
  ASTNodeType type() { return ASTNodeType::Symbol; }
  bool getBool() { return true; }
  std::string getRepr() { return symbol; }
  ASTNode* deepCopy() { return new SymbolNode(symbol); }
  std::string symbol;
};

class LambdaNode : public ASTNode {
 public:
  LambdaNode(ASTNode* x, ASTNode* y) : arglist(x), body(y) {}
  ~LambdaNode() {
    delete arglist;
    delete body;
  }
  ASTNodeType type() { return ASTNodeType::Lambda; }
  bool getBool() { return true; }
  std::string getRepr() {
    return std::string("<lambda=") + std::to_string((uint64_t)this) + ">";
  }
  ASTNode* deepCopy() {
    return new LambdaNode(arglist->deepCopy(), body->deepCopy());
  }
  ASTNode* arglist = nullptr;
  ASTNode* body = nullptr;
};

class ListNode : public ASTNode {
 public:
  ListNode(std::list<ASTNode*> l) : list(l) {}
  ~ListNode() {
    for (auto node : list) delete node;
  }
  ASTNodeType type() { return ASTNodeType::List; }
  bool getBool() { return (list.size() != 0); }
  std::string getRepr() {
    std::string lr = "( ";
    for (auto it = list.begin(); it != list.end(); it++) {
      lr += (*it)->getRepr();
      lr += " ";
    }
    lr += ")";
    return lr;
  }
  ASTNode* deepCopy() {
    std::list<ASTNode*> retlist;
    for (auto& node : list) {
      retlist.push_back(node->deepCopy());
    }
    return new ListNode(retlist);
  }
  std::list<ASTNode*> list;
};
