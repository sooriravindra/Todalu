#include <cstdint>
#include <list>
typedef struct _IRNode {
  uint8_t type;
  int64_t value;
} IRNode;

typedef struct _LambdaStruct {
  std::list<IRNode*>* arglist;
  IRNode* (*fun)();
  int argc;
} LambdaStruct;

IRNode* add(int num_args, ...);
