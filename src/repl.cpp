#include <iostream>
#include "linenoise.h"
using namespace std;
int main() {
  char* line;
  while ((line = linenoise("\n> ")) != nullptr) {
    cout << line;
    linenoiseFree(line); 
  }
}
