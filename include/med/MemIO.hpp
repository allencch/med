#ifndef MED_MEM_IO_HPP
#define MED_MEM_IO_HPP

#include <mutex>
#include <sys/types.h>
#include "med/MedTypes.hpp"
#include "med/SizedBytes.hpp"

class MemIO {
public:
    explicit MemIO(pid_t pid = 0);
    ~MemIO();

    void setPid(pid_t pid);
    pid_t getPid() const;

    SizedBytes read(Address addr, size_t size);
    void write(Address addr, const SizedBytes& data);
    void write(Address addr, const Byte* data, size_t size);

private:
    SizedBytes readDirect(Address addr, size_t size);
    SizedBytes readProcess(Address addr, size_t size);

    void writeDirect(Address addr, const Byte* data, size_t size);
    void writeProcess(Address addr, const Byte* data, size_t size);

    int getFd();
    void closeFd();

    pid_t pid_ = 0;
    int fd_ = -1;
    mutable std::mutex mutex_;
};

#endif
