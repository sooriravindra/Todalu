#include <regex>
#include <string>
#include <list>
#include "common.h"
#include "granthalaya.h"

void Inpiler::load_granthalaya() {
  std::string str((char*)granthalaya_tdl, granthalaya_tdl_len);
  std::istringstream is(str);
  std::string line;
  std::string wholeline = "";
  while (std::getline(is, line)) {
    wholeline += line;
    if (is_balanced(wholeline)) {
      handle_line(wholeline);
      wholeline = "";
    }
  }
}

bool is_comment(std::string& line) {
  for (auto c : line) {
    if (std::isspace(c)) continue;
    if (c == '#') return true;
    return false;
  }
  return true;
}

std::list<std::string> tokenizer(std::string& str) {
  str = std::regex_replace(str, std::regex("\n"), " ");
  str = std::regex_replace(str, std::regex("\\("), " ( ");
  str = std::regex_replace(str, std::regex("\\)"), " ) ");
  std::list<std::string> tokens;
  std::stringstream ss(str);
  std::string token;
  while (ss >> token) {
    tokens.push_back(token);
  }
  return tokens;
}
