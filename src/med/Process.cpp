#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <signal.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

#include "med/Process.hpp"
#include "med/MedException.hpp"

Process::Process(pid_t pid, std::string cmdline) : pid_(pid), cmdline_(std::move(cmdline)) {}

pid_t Process::getPid() const {
    return pid_;
}

const std::string& Process::getCmdline() const {
    return cmdline_;
}

std::vector<Process> Process::listAll() {
    std::vector<Process> pids;
    DIR* d = opendir("/proc");
    if (d) {
        struct dirent* dir;
        while ((dir = readdir(d)) != nullptr) {
            if (std::isdigit(dir->d_name[0])) {
                pid_t pid = std::atoi(dir->d_name);
                std::string cmd = getName(pid);
                if (!cmd.empty()) {
                    pids.emplace_back(pid, cmd);
                }
            }
        }
        closedir(d);
    }
    return pids;
}

std::string Process::getName(pid_t pid) {
    std::string ret;
    std::string path = "/proc/" + std::to_string(pid) + "/cmdline";
    std::ifstream ifile(path);
    if (ifile.fail()) {
        return "";
    }
    std::getline(ifile, ret);
    ifile.close();
    return ret;
}

Maps Process::getMaps() const {
    Maps maps;
    std::string filename = "/proc/" + std::to_string(pid_) + "/maps";
    FILE* file = fopen(filename.c_str(), "r");
    if (!file) {
        throw MedException("Failed to open maps: " + filename);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        unsigned long start, end;
        char rd, wr;
        // Example line: 559902f43000-559902f45000 r--p 00000000 00:1c 19280145                   /usr/bin/cat
        if (sscanf(line, "%lx-%lx %c%c", &start, &end, &rd, &wr) < 4) {
            continue;
        }

        if (rd == 'r' && wr == 'w' && (end > start)) {
            maps.push({(Address)start, (Address)end});
        }
    }

    fclose(file);
    return maps;
}

bool Process::isSuspended() const {
    std::string filename = "/proc/" + std::to_string(pid_) + "/stat";
    std::ifstream ifile(filename);
    if (!ifile) {
        return false;
    }
    std::string line;
    std::getline(ifile, line);
    ifile.close();

    std::regex reg("\\d+ \\(.*?\\) (\\w) \\d+");
    std::smatch m;
    if (std::regex_search(line, m, reg)) {
        char state = m.str(1).at(0);
        return state == 'T' || state == 't';
    }
    return false;
}

void Process::resume() const {
    if (kill(pid_, SIGCONT) == -1) {
        throw MedException("Failed to resume process: " + std::to_string(pid_));
    }
}

void Process::stop() const {
    if (kill(pid_, SIGSTOP) == -1) {
        throw MedException("Failed to stop process: " + std::to_string(pid_));
    }
}

void Process::attach() const {
    if (ptrace(PTRACE_ATTACH, pid_, nullptr, nullptr) == -1) {
        throw MedException("Failed to attach to process: " + std::to_string(pid_));
    }

    int status;
    if (waitpid(pid_, &status, 0) == -1 || !WIFSTOPPED(status)) {
        throw MedException("Failed to wait for process after attach: " + std::to_string(pid_));
    }
}

void Process::detach() const {
    if (ptrace(PTRACE_DETACH, pid_, nullptr, nullptr) == -1) {
        throw MedException("Failed to detach from process: " + std::to_string(pid_));
    }
}
