#ifndef MED_COMMON_HPP
#define MED_COMMON_HPP

#include <string>
#include <mutex>

#include "med/MedTypes.hpp"
#include "med/MedAddress.hpp"

using namespace std;

class Process;

/**
 * Convert string to ScanType, they are "int8", "int16", etc.
 */
ScanType stringToScanType(string scanType);

/**
 * Convert ScanType to sizeof
 */
int scanTypeToSize(ScanType type);
int scanTypeToSize(string type);

string scanTypeToString(ScanType scanType);

/**
 * @brief Convert hexadecimal string to integer value
 */
long hexToInt(string str);

/**
 * @brief Convert long integer to hex string
 */
string intToHex(long int);

/**
 * @brief Print the hexadecimal data
 */
void printHex(FILE* file, void* addr, int size);

/**
 * @param pid is pid_t, which is actually integer.
 */
ProcMaps getMaps(pid_t pid);

// Create buffer by scan type. Remember to free()
int createBufferByScanType(ScanType type, void** buffer, int size = 1);

/**
 * Convert numerical string to raw data (hexadecimal).
 * @param s is a string which can separated by space
 * @param buffer is to store the output address, must be free() if not used.
 * @return number of values to be scanned
 */
int stringToRaw(string str, ScanType type, Byte** buffer);

/**
 * @param type is string in "int8", "int16", etc
 */
int stringToRaw(string str, string type, Byte** buffer);

int stringToRawNumericType(string str, ScanType type, Byte** buffer);
int stringToRawStringType(string str, ScanType type, Byte** buffer);

/**
 * Convert the size to padded word size.
 */
int padWordSize(int x);

/**
 * Open the /proc/[pid]/mem
 * @return file descriptor
 */
int getMem(pid_t pid);

pid_t pidAttach(pid_t pid);
pid_t pidDetach(pid_t pid);


/**
 * Get the cmdline from PID
 */
string pidName(string pid);

vector<Process> pidList();

/**
 * Lock value thread
 */
void lockValue(string pid, MedAddress* address);

/**
 * This will just perform the unlock by force
 */
void tryUnlock(std::mutex& mutex);

void stringToMemory(const string& str, const string& type, Byte* buffer); // Similar to stringToRaw, but this doesn't create new memory
void stringToMemory(const string& str, const ScanType& type, Byte* buffer);

#endif
