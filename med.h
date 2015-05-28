/*************
Copyright (c) 2012, Allen Choong
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the project nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************/

/**
 * @author	Allen Choong
 * @version	0.0.1
 * @date	2012-03-15
 *
 * Simple memory editor. The goal is to hack the Android game from adb shell
 * It is inspired by scanmem.
 * Actually intend to write in C, but I need list (vector), so I change
 * it to C++.
 * By default, all the search is little endian.
 *
 * Todo:
 * Scan integer, float, little endian, big endian, arrays
 * Edit integer, float, little endian, big endian, arrays
 * Dump with hex position
 * In the interactive mode, 
 * try to use solve the keyboard problem, such as moving cursor.
 * Pause and play the process
 *
 *
 * Search by block
 * 
 *
 * Fixed:
 * Solve the problem of the I/O Error for the large running file
 * On Android, the scan and edit does not work, because of permission limit.
 * Tried the signal.h kill() to SIGSTOP the process during scanning in Android, but still get the EIO
 * (I/O Error). 
 *
 * Important:
 * pidAttach() must be attach only when scan, else the process is being paused.
 * Regarding "word" in ptrace(), the size of "word" is depending on the architecture. But ptrace() itself is using long. So, "long" datatype is the solution for the word size.
 */


#ifndef MED_H
#define MED_H

#include <string>

using namespace std;

struct ProcMaps {
	vector<unsigned long> starts;
	vector<unsigned long> ends;
};

struct Process {
	string pid;
	string cmdline;
};

struct Scanner {
	vector<unsigned long> addresses;
}; 

enum ScanType {
	Int8,
	Int16,
	Int32,
	Float32,
	Float64,
	Unknown
};


/**
 * Convert string to ScanType, they are "int8", "int16", etc.
 */
ScanType stringToScanType(string scanType);


/**
 * Convert ScanType to sizeof
 */
int scanTypeToSize(ScanType type);


/**
 * @brief Convert hexadecimal string to integer value
 */
long hexToInt(string str) throw(string);

/**
 * @brief Convert long integer to hex string
 */
string intToHex(long int);


/**
 * @brief Print the hexadecimal data
 */
void printHex(FILE* file,void* addr,int size);

/**
 * @param pid is pid_t, which is actually integer.
 */
ProcMaps getMaps(pid_t pid);

/**
 * Convert numerical string to raw data (hexadecimal).
 * @param s is a string which can separated by space
 * @param buffer is to store the output address, must be free() if not used.
 * @return number of values to be scanned
 */
int stringToRaw(string str, ScanType type, uint8_t** buffer);

/**
 * @param type is string in "int8", "int16", etc
 */
int stringToRaw(string str, string type, uint8_t** buffer);


/**
 * Convert the size to padded word size.
 */
int padWordSize(int x);

/**
 * Open the /proc/[pid]/mem
 * @return file descriptor
 */
int getMem(pid_t pid);

pid_t pidAttach(pid_t pid) throw(string);
pid_t pidDetach(pid_t pid) throw(string);

int memDump(pid_t pid,unsigned long address,int size);
void memWrite(pid_t pid,unsigned long address,uint8_t* data,int size);
void memWriteList(Scanner scanner,pid_t pid,uint8_t* data,int size);

int memCopy(pid_t pid,unsigned long address,unsigned char* out,int size,bool showError);

/**
 * Scan memory, using procfs mem
 * @param pid (pid_t) is an integer of PID
 */
void memScanEqual(Scanner &scanner,pid_t pid,unsigned char* data,int size);
void memScanFilter(Scanner &scanner,pid_t pid,unsigned char* data,int size);

/**
 * Reverse the memory (Big to Little Endian or vice versa)
 */
void memReverse(uint8_t* buf,int size);

/**
 * Get the list of PID
 * This is done by accessing the /proc and /proc/PID and /proc/PID/cmdline
 * The list suppose to be in the descending order
 */
vector<Process> pidList();

/**
 * Get the cmdline from PID
 */
string pidName(string pid);

/**
 * Get the value from the address as string based on the scanType (string)
 */
string memValue(long pid, unsigned long address, string scanType) throw (string);

#endif