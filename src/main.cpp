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
 */


//#define _FILE_OFFSET_BITS 64


#include <cstdio>
#include <cstdlib>
#include <cstdint> //uint8

#include <cerrno>  //errno
#include <cstring> //strerror()


#include <sys/ptrace.h> //ptrace()
#include <sys/wait.h> //waitpid()
#include <linux/capability.h>
#include <sys/prctl.h> //prctl()
#include <unistd.h> //getpagesize()
#include <fcntl.h> //open, read, lseek

#include <string>
#include <iostream>
#include <sstream>
#include <iterator>
#include <vector>

#include "med.hpp"

Scanner scanner;


int main(int argc,char** argv) {
	pid_t targetPid = atoi(argv[1]);

	//Enter interactive mode
	string command;
	char buffer[64];
	while(command != "q") {
		cout<<"> ";
		cin.getline(buffer,63);
		if(strlen(buffer) == 0)
			continue;

		//Tokenise the command
		stringstream ss(buffer);
		istream_iterator<string> eos; //end of string
		istream_iterator<string> iit(ss);

		vector<string> tokens(iit,eos);

		command = tokens[0];


		if(command == "q") { //Quit
			break;
		}


		//Search

		else if(command == "s") { //Search the hexadecimal string, prefix with 0x, that is the exact pattern
			//Get the following arguments size
			int size = tokens.size() -1;

			//Create buffer
			unsigned char* buf = (unsigned char*)malloc(size);

			//Convert following arguments into buffer
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				int temp;
				if(!(ss >> hex >> temp)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
				buf[i-1] = (unsigned char)temp;
			}

			//Search for the value
			memScanEqual(scanner,targetPid,buf,size);

			free(buf);
		}

		else if(command == "si") { //Search 32bit integer
			int size = tokens.size() -1;
			uint8_t* buf = (uint8_t*)malloc(sizeof(int)*size);
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(int*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}

			memScanEqual(scanner,targetPid,buf,size*sizeof(int));

			free(buf);
		}

		else if(command == "sir") { //Search 32bit integer with reverse (big endian)
			int size = tokens.size() -1;
			uint8_t* buf = (uint8_t*)malloc(sizeof(int)*size);
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(int*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}

			memReverse(buf,sizeof(int)*size);
			memScanEqual(scanner,targetPid,buf,size*sizeof(int));

			free(buf);
		}

		else if(command == "sf") { //Search 32bit float
			int size = tokens.size() -1;
			uint8_t* buf = (uint8_t*)malloc(sizeof(float)*size);
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(float*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}

			memScanEqual(scanner,targetPid,buf,size*sizeof(float));

			free(buf);
		}

		else if(command == "sd") { //Search double
			int size = tokens.size() -1;
			uint8_t* buf = (uint8_t*)malloc(sizeof(double)*size);
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(double*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}

			memScanEqual(scanner,targetPid,buf,size*sizeof(double));

			free(buf);
		}

		//Filter

		else if(command == "f") {//Filter the searched pattern
			//Get the following arguments size
			int size = tokens.size() -1;

			//Create buffer
			unsigned char* buf = (unsigned char*)malloc(size);


			//Convert following arguments into buffer
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				int temp;
				if(!(ss >> hex >> temp)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
				buf[i-1] = (unsigned char)temp;
			}


			//Search for the value
			memScanFilter(scanner,targetPid,buf,size);

			free(buf);
		}

		else if(command == "fi") { //Search 32bit integer
			int size = tokens.size() -1;
			uint8_t* buf = (uint8_t*)malloc(sizeof(int)*size);
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(int*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}

			memScanFilter(scanner,targetPid,buf,size*sizeof(int));

			free(buf);
		}

		else if(command == "fir") { //
			int size = tokens.size() -1;
			uint8_t* buf = (uint8_t*)malloc(sizeof(int)*size);
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(int*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}
			memReverse(buf,sizeof(int)*size);
			memScanFilter(scanner,targetPid,buf,size*sizeof(int));

			free(buf);
		}


		else if(command == "ff") { //Filter float
			int size = tokens.size() -1;
			uint8_t* buf = (uint8_t*)malloc(sizeof(float)*size);
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(float*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}

			memScanFilter(scanner,targetPid,buf,size*sizeof(float));

			free(buf);
		}

		else if(command == "fd") { //Filter double
			int size = tokens.size() -1;
			uint8_t* buf = (uint8_t*)malloc(sizeof(double)*size);
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(double*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}

			memScanFilter(scanner,targetPid,buf,size*sizeof(double));

			free(buf);
		}


		//Edit

		else if(command == "e") {  //Edit, 1 argument is the hexadecimal address, prefix with 0x, 2nd is the string hexadecimal
			long int address = hexToInt(tokens[1]);

			//Get the following arguments size
			int size = tokens.size() -2;

			//Create buffer
			unsigned char* buf = (unsigned char*)malloc(size);

			//Convert following arguments into buffer
			for(int i=2;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				int temp;
				if(!(ss >> hex >> temp)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}

				buf[i-2] = (unsigned char)temp;
			}

			//Search for the value
			memWrite(targetPid,address,buf,size);

			free(buf);//*/
		}

		else if(command == "ei") {  //Edit by writing with integer
			long int address = hexToInt(tokens[1]);

			//Get the following arguments size
			int size = tokens.size() -2;

			//Create buffer
			uint8_t* buf = (uint8_t*)malloc(sizeof(int)*size);

			//Convert following arguments into buffer
			for(int i=2;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(int*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}

			//Search for the value
			memWrite(targetPid,address,buf,size*sizeof(int));

			free(buf);//*/
		}

		else if(command == "eil") {  //Edit by writing all item in list with integer, take your own risk
			//Get the following arguments size
			int size = tokens.size() -1;

			//Create buffer
			uint8_t* buf = (uint8_t*)malloc(sizeof(int)*size);

			//Convert following arguments into buffer
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(int*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}

			//FIXME memwriteList
			memWriteList(scanner,targetPid,buf,size*sizeof(int));

			free(buf);//*/
		}

		else if(command == "ef") {  //Edit by writing with float
			long int address = hexToInt(tokens[1]);

			//Get the following arguments size
			int size = tokens.size() -2;

			//Create buffer
			uint8_t* buf = (uint8_t*)malloc(sizeof(float)*size);

			//Convert following arguments into buffer
			for(int i=2;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(float*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}

			//Search for the value
			memWrite(targetPid,address,buf,size*sizeof(float));

			free(buf);//*/
		}

		else if(command == "efl") {  //Edit by writing all item in list with integer, take your own risk
			//Get the following arguments size
			int size = tokens.size() -1;

			//Create buffer
			uint8_t* buf = (uint8_t*)malloc(sizeof(float)*size);

			//Convert following arguments into buffer
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(float*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}

			//Search for the value
			memWriteList(scanner,targetPid,buf,size*sizeof(float));

			free(buf);//*/
		}



		else if(command == "ed") {  //Edit by writing with integer
			long int address = hexToInt(tokens[1]);

			//Get the following arguments size
			int size = tokens.size() -2;

			//Create buffer
			uint8_t* buf = (uint8_t*)malloc(sizeof(double)*size);

			//Convert following arguments into buffer
			for(int i=2;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(double*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}

			}

			//Search for the value
			memWrite(targetPid,address,buf,size*sizeof(double));

			free(buf);//*/
		}

		else if(command == "edl") {  //Edit by writing all item in list with integer, take your own risk
			//Get the following arguments size
			int size = tokens.size() -1;

			//Create buffer
			uint8_t* buf = (uint8_t*)malloc(sizeof(double)*size);

			//Convert following arguments into buffer
			for(int i=1;i<tokens.size();i++) {
				stringstream ss(tokens[i]);
				if(!(ss >> dec >> *(double*)buf)) {
					printf("Error input: %s\n",tokens[i].c_str());
				}
			}

			//Search for the value
			memWriteList(scanner,targetPid,buf,size*sizeof(double));

			free(buf);//*/
		}


		//Display/Dump
		// Where the second parameter can be a string with / without "0x", but a hexadecimal string, and the 3rd parameter is the size in integer
		else if(command == "d") {//Dump
			if(tokens.size() < 3 ) {
				printf("Missing arguments\n");
				continue;
			}

			long int address = hexToInt(tokens[1]);

			//Get the size which want to print
			int size = hexToInt(tokens[2]);

			memDump(targetPid,address,size);

		}
		else if(command == "l") { //List the searched

			for(int i=0;i<scanner.addresses.size();i++) {
				printf("%p\t",scanner.addresses[i]);
				memDump(targetPid,scanner.addresses[i],10);
			}
			printf("End of list\n");
		}

	}

	return 0;
}
