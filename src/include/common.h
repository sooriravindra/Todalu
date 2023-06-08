#ifndef _COMMONH
#define _COMMONH
#include <exception>
#include <stdexcept>
#include <list>
#include <string>
#include <sstream>

class Inpiler {
 public:
  virtual std::string handle_line(std::string str) = 0;
  virtual ~Inpiler() {}
  bool is_balanced(std::string& line) {
    int count = 0;
    for (auto c : line) {
      if (c == '(')
        count++;
      else if (c == ')')
        count--;
    }
    return (count <= 0);
  }
  void load_granthalaya();
};

class TodaluException : public std::runtime_error {
 public:
  explicit TodaluException(const std::string& msg) : std::runtime_error(msg) {}
};

bool is_comment(std::string& line);
std::list<std::string> tokenizer(std::string& str);
#endif
