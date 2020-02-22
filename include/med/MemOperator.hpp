#ifndef MEM_OPERATOR_H
#define MEM_OPERATOR_H

#include <string>
#include <vector>

#include "med/MedTypes.hpp"
#include "med/MedException.hpp"
#include "med/ScanParser.hpp"
#include "med/Operands.hpp"

using namespace std;

struct Scanner {
  vector<Address> addresses;
};


void memDump(pid_t pid, Address address, int size);
void memDirectDump(Byte* address, int size);

/**
 * Reverse the memory (Big to Little Endian or vice versa)
 */
void memReverse(uint8_t* buf,int size);


// Memory comparison function
bool memEq(const void* ptr1, const void* ptr2, size_t size);
bool memGt(const void* ptr1, const void* ptr2, size_t size);
bool memLt(const void* ptr1, const void* ptr2, size_t size);
bool memNeq(const void* ptr1, const void* ptr2, size_t size);
bool memGe(const void* ptr1, const void* ptr2, size_t size); //Greater or equal
bool memLe(const void* ptr1, const void* ptr2, size_t size);

/**
 * Compare the memory based on the operation
 */
bool memCompare(const void* ptr1, const void* ptr2, size_t size, ScanParser::OpType op);
bool memCompare(const void* ptr1, size_t size1, const void* ptr2, size_t size2, ScanParser::OpType op);
bool memCompare(const void* ptr, size_t size, Operands& operands, const ScanParser::OpType& op);


/**
 * Compare the value of a memory address with two other memory addresses whether it is within the interval [low, up]
 */
bool memWithin(const void* src, const void* low, const void* up, size_t size);

string memToString(Byte* memory, string scanType);

Address addressRoundDown(Address addr);

#endif
