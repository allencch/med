#ifndef MEM_OPERATOR_H
#define MEM_OPERATOR_H

#include <string>
#include <vector>

#include "med.hpp"
#include "MedException.hpp"
#include "ScanParser.hpp"

using namespace std;

struct Scanner {
  vector<MemAddr> addresses;
};


int memDump(pid_t pid, MemAddr address,int size);
void memWrite(pid_t pid, MemAddr address,uint8_t* data,int size);

/**
 * @deprecated
 */
void memWriteList(Scanner scanner,pid_t pid,uint8_t* data,int size);

/**
 * Reverse the memory (Big to Little Endian or vice versa)
 */
void memReverse(uint8_t* buf,int size);

/**
 * Get the value from the address as string based on the scanType (string)
 */
string memValue(long pid, MemAddr address, string scanType) throw(MedException);

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


/**
 * Compare the value of a memory address with two other memory addresses whether it is within the interval [low, up]
 */
bool memWithin(const void* src, const void* low, const void* up);


#endif
