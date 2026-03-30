#ifndef MED_EXCEPTION_H
#define MED_EXCEPTION_H

#include <string>
#include <exception>

using namespace std;

class MedException: public std::nested_exception {
public:
  explicit MedException(const string& message) : message(message) { }
  virtual const char* what() const throw() {
    return message.c_str();
  }

  string getMessage() {
    return message;
  }

private:
  string message;
};

class EmptyListException : public MedException {
public:
  explicit EmptyListException(const string& message) : MedException(message) {}
};

#endif
