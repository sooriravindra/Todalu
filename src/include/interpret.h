#include "ast.h"
#include "common.h"
#include <string>

class Interpreter : public Inpiler {
public:
  ~Interpreter();
  std::string handle_line(std::string str);
private:
  ASTNode* eval_tree(ASTNode* node);
  void unbind_arguments(LambdaNode* lambda);
  void bind_arguments(LambdaNode* lambda, ListNode* listnode);
  void operate_on_node(ASTNode* node, char op, float& acc, bool& is_all_int);
};
