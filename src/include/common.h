#include <exception>
#include <stdexcept>
#include <string>

class TodaluException : public std::runtime_error {
 public:
  explicit TodaluException(const std::string& msg) : std::runtime_error(msg) {}
};
