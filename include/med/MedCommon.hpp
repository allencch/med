#ifndef MED_COMMON_HPP
#define MED_COMMON_HPP

#include <string>
#include <mutex>

#include "med/MedTypes.hpp"
#include "med/Process.hpp"
#include "mem/Maps.hpp"

using namespace std;

/**
 * Convert string to ScanType, they are "int8", "int16", etc.
 */
ScanType stringToScanType(const string& scanType);

/**
 * Convert ScanType to sizeof
 */
int scanTypeToSize(const ScanType& type);
int scanTypeToSize(const string& type);

string scanTypeToString(const ScanType& scanType);

int hexStrToInt(const string& str);

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
Maps getMaps(pid_t pid);

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
bool isPidSuspended(pid_t pid);
int pidResume(pid_t pid);
int pidStop(pid_t pid);

/**
 * Get the cmdline from PID
 */
string pidName(const string& pid);

vector<Process> pidList();

/**
 * This will just perform the unlock by force
 */
void tryUnlock(std::mutex& mutex);

void stringToMemory(const string& str, const string& type, Byte* buffer); // Similar to stringToRaw, but this doesn't create new memory
void stringToMemory(const string& str, const ScanType& type, Byte* buffer);

#endif
