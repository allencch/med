#ifndef MED_EXCEPTION_HPP
#define MED_EXCEPTION_HPP

#include <string>
#include <exception>

class MedException : public std::exception {
public:
    explicit MedException(std::string message) : message_(std::move(message)) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }

    const std::string& getMessage() const {
        return message_;
    }

private:
    std::string message_;
};

class EmptyListException : public MedException {
public:
    explicit EmptyListException(const std::string& message) : MedException(message) {}
};

#endif
