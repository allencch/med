#ifndef STRING_UTIL_HPP
#define STRING_UTIL_HPP

#include <string>
#include <vector>

using namespace std;

namespace StringUtil {
  string trim(const string& s);
  vector<string> split(const string& s, char delim);
  string toLower(const string& s);
  string replace(string& s, const string& find, const string& r);
}

#endif
