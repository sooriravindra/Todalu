#include <list>
#include <string>

enum class ASTNodeType { Bool, Integer, Decimal, Symbol, List, Lambda, String };

class ASTNode {
 public:
  virtual std::string getRepr() const = 0;
  virtual bool getBool() const = 0;
  virtual ASTNodeType type() const = 0;
  virtual ASTNode* deepCopy() const { return nullptr; };
  virtual ~ASTNode() {}
};

class BoolNode : public ASTNode {
 public:
  BoolNode(bool v) : value(v) {}
  ASTNodeType type() const { return ASTNodeType::Bool; }
  bool getBool() const { return value; }
  std::string getRepr() const {
    if (value) return "#true";
    return "#false";
  }
  ASTNode* deepCopy() const { return new BoolNode(value); }
  bool value = false;
};

class IntegerNode : public ASTNode {
 public:
  IntegerNode(int v) : value(v) {}
  ASTNodeType type() const { return ASTNodeType::Integer; }
  bool getBool() const { return (value != 0); }
  std::string getRepr() const { return std::to_string(value); }
  ASTNode* deepCopy() const { return new IntegerNode(value); }
  int value = 0;
};

class DecimalNode : public ASTNode {
 public:
  DecimalNode(float v) : value(v) {}
  ASTNodeType type() const { return ASTNodeType::Decimal; }
  bool getBool() const { return (value != 0); }
  std::string getRepr() const { return std::to_string(value); }
  ASTNode* deepCopy() const { return new DecimalNode(value); }
  float value = 0;
};

class StringNode : public ASTNode {
 public:
  StringNode(std::string v) : value(v) {}
  ASTNodeType type() const { return ASTNodeType::String; }
  bool getBool() const { return !value.empty(); }
  std::string getRepr() const {
    std::string repr = "\"";
    repr += value;
    repr += "\"";
    return repr;
  }
  ASTNode* deepCopy() const { return new StringNode(value); }
  std::string value = "";
};

class SymbolNode : public ASTNode {
 public:
  SymbolNode(std::string v) : symbol(v) {}
  ASTNodeType type() const { return ASTNodeType::Symbol; }
  bool getBool() const { return true; }
  std::string getRepr() const { return symbol; }
  ASTNode* deepCopy() const { return new SymbolNode(symbol); }
  std::string symbol;
};

class LambdaNode : public ASTNode {
 public:
  LambdaNode(ASTNode* x, ASTNode* y) : arglist(x), body(y) {}
  ~LambdaNode() {
    delete arglist;
    delete body;
  }
  ASTNodeType type() const { return ASTNodeType::Lambda; }
  bool getBool() const { return true; }
  std::string getRepr() const {
    return std::string("<lambda=") + std::to_string((uint64_t)this) + ">";
  }
  ASTNode* deepCopy() const {
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
  ASTNodeType type() const { return ASTNodeType::List; }
  bool getBool() const { return (list.size() != 0); }
  std::string getRepr() const {
    std::string lr = "( ";
    for (auto it = list.begin(); it != list.end(); it++) {
      lr += (*it)->getRepr();
      lr += " ";
    }
    lr += ")";
    return lr;
  }
  ASTNode* deepCopy() const {
    std::list<ASTNode*> retlist;
    for (auto& node : list) {
      retlist.push_back(node->deepCopy());
    }
    return new ListNode(retlist);
  }
  std::list<ASTNode*> list;
};

std::list<ASTNode*> create_ast(std::list<std::string>& tokens,
                               bool closing_paren_allow = false);
