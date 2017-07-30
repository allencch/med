#ifndef MED_EXCEPTION_H
#define MED_EXCEPTION_H

#include <string>

using namespace std;

class MedException: public exception {
public:
  MedException(string message) {
    this->message = message;
  }
  virtual const char* what() const throw() {
    return message.c_str();
  }

  string getMessage() {
    return message;
  }

private:
  string message;
};

#endif
