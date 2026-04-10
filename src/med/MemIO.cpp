#include <cstring>
#include <iostream>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include "med/MemIO.hpp"
#include "med/MedException.hpp"

MemIO::MemIO(pid_t pid) : pid_(pid) {}

MemIO::~MemIO() {
    if (fd_ != -1) {
        close(fd_);
    }
}

void MemIO::setPid(pid_t pid) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (pid_ != pid) {
        if (fd_ != -1) {
            close(fd_);
            fd_ = -1;
        }
        pid_ = pid;
    }
}

pid_t MemIO::getPid() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pid_;
}

SizedBytes MemIO::read(Address addr, size_t size) {
    pid_t currentPid;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        currentPid = pid_;
    }

    if (currentPid == 0) {
        return readDirect(addr, size);
    }
    return readProcess(addr, size);
}

void MemIO::write(Address addr, const SizedBytes& data) {
    write(addr, data.getBytes(), data.getSize());
}

void MemIO::write(Address addr, const Byte* data, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (pid_ == 0) {
        writeDirect(addr, data, size);
    } else {
        writeProcess(addr, data, size);
    }
}

SizedBytes MemIO::readDirect(Address addr, size_t size) {
    return SizedBytes((Byte*)addr, size);
}

SizedBytes MemIO::readProcess(Address addr, size_t size) {
    SizedBytes res = SizedBytes::create(size);
    struct iovec local[1];
    struct iovec remote[1];

    local[0].iov_base = res.getBytes();
    local[0].iov_len = size;
    remote[0].iov_base = (void*)addr;
    remote[0].iov_len = size;

    pid_t currentPid;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        currentPid = pid_;
    }

    ssize_t nread = process_vm_readv(currentPid, local, 1, remote, 1, 0);
    if (nread == -1) {
        // Fallback to /proc/[pid]/mem if process_vm_readv fails (e.g. permissions)
        // HOLD the lock for the ENTIRE fallback: open + pread atomically
        std::lock_guard<std::mutex> lock(mutex_);
        if (fd_ == -1 && pid_ != 0) {
            std::string filename = "/proc/" + std::to_string(pid_) + "/mem";
            fd_ = open(filename.c_str(), O_RDONLY);
        }
        if (fd_ == -1) {
            throw MedException("Failed to open /proc/pid/mem");
        }
        if (pread(fd_, res.getBytes(), size, addr) == -1) {
            throw MedException("Failed to read from /proc/pid/mem at " + std::to_string(addr));
        }
    } else if ((size_t)nread != size) {
        throw MedException("Partial read from process memory");
    }

    return res;
}

int MemIO::getFd() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fd_ != -1) return fd_;
    if (pid_ == 0) return -1;

    std::string filename = "/proc/" + std::to_string(pid_) + "/mem";
    fd_ = open(filename.c_str(), O_RDONLY);
    return fd_;
}

void MemIO::closeFd() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fd_ != -1) {
        close(fd_);
        fd_ = -1;
    }
}

void MemIO::writeDirect(Address addr, const Byte* data, size_t size) {
    std::memcpy((void*)addr, data, size);
}

void MemIO::writeProcess(Address addr, const Byte* data, size_t size) {
    struct iovec local[1];
    struct iovec remote[1];

    local[0].iov_base = (void*)data;
    local[0].iov_len = size;
    remote[0].iov_base = (void*)addr;
    remote[0].iov_len = size;

    ssize_t nwrite = process_vm_writev(pid_, local, 1, remote, 1, 0);
    if (nwrite == -1) {
        // Fallback to ptrace if process_vm_writev fails
        // Note: writing to /proc/[pid]/mem is often not allowed even for root
        // So we use ptrace as fallback.
        // We'll use the ptrace logic from old code but cleaner.
        
        if (ptrace(PTRACE_ATTACH, pid_, nullptr, nullptr) == -1) {
            throw MedException("Failed to attach for writing");
        }
        int status;
        waitpid(pid_, &status, 0);

        for (size_t i = 0; i < size; i += sizeof(long)) {
            size_t remaining = size - i;
            long word;
            if (remaining < sizeof(long)) {
                // Need to peek first
                word = ptrace(PTRACE_PEEKDATA, pid_, (void*)(addr + i), nullptr);
                std::memcpy(&word, data + i, remaining);
            } else {
                std::memcpy(&word, data + i, sizeof(long));
            }

            if (ptrace(PTRACE_POKEDATA, pid_, (void*)(addr + i), (void*)word) == -1) {
                ptrace(PTRACE_DETACH, pid_, nullptr, nullptr);
                throw MedException("Failed POKEDATA at " + std::to_string(addr + i));
            }
        }
        ptrace(PTRACE_DETACH, pid_, nullptr, nullptr);
    } else if ((size_t)nwrite != size) {
        throw MedException("Partial write to process memory");
    }
}
