#include <exception>
#include <stdexcept>
#include <string>

class ParseError : public std::runtime_error {
 public:
  explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

std::string interpret_line(std::string str);
