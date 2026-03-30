#ifndef MED_PROCESS_HPP
#define MED_PROCESS_HPP

#include <string>
#include <vector>
#include <sys/types.h>
#include "med/MedTypes.hpp"
#include "med/Maps.hpp"

class Process {
public:
    Process() = default;
    Process(pid_t pid, std::string cmdline);

    pid_t getPid() const;
    const std::string& getCmdline() const;

    static std::vector<Process> listAll();
    static std::string getName(pid_t pid);

    Maps getMaps() const;

    bool isSuspended() const;
    void resume() const;
    void stop() const;

    void attach() const;
    void detach() const;

private:
    pid_t pid_ = 0;
    std::string cmdline_;
};

#endif
