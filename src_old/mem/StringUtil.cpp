#include <sstream>
#include "mem/StringUtil.hpp"

using namespace std;

string StringUtil::trim(const string& s) {
  if (s.size() == 0) {
    return s;
  }
  size_t first = s.find_first_not_of(' ');
  size_t last = s.find_last_not_of(' ');
  return s.substr(first, (last - first + 1));
}

vector<string> StringUtil::split(const string& s, char delim) {
  stringstream ss(s);
  string token;
  vector<string> tokens;
  while (getline(ss, token, delim)) {
    tokens.push_back(StringUtil::trim(token));
  }
  return tokens;
}

string StringUtil::toLower(const string& s) {
  string newString = s;
  for (char &ch : newString){
    ch = std::tolower(ch);
  }
  return newString;
}

string StringUtil::replace(string& s, const string& find, const string& r) {
  if (!find.length()) {
    return s;
  }
  string::size_type n = s.find(find);
  if (n == string::npos) {
    return s;
  }
  return s.replace(n, find.length(), r);
}
