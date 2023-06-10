#include <cstdint>
typedef struct _IRNode {
  uint8_t type;
  int64_t value;
} IRNode;

IRNode* add(int num_args, ...);
