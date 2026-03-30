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

void MemIO::setPid(pid_t pid) {
    std::lock_guard<std::mutex> lock(mutex_);
    pid_ = pid;
}

pid_t MemIO::getPid() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pid_;
}

SizedBytes MemIO::read(Address addr, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (pid_ == 0) {
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

    ssize_t nread = process_vm_readv(pid_, local, 1, remote, 1, 0);
    if (nread == -1) {
        // Fallback to /proc/[pid]/mem if process_vm_readv fails (e.g. permissions)
        std::string filename = "/proc/" + std::to_string(pid_) + "/mem";
        int fd = open(filename.c_str(), O_RDONLY);
        if (fd == -1) {
            throw MedException("Failed to open " + filename);
        }
        if (lseek(fd, addr, SEEK_SET) == -1) {
            close(fd);
            throw MedException("Failed to lseek in " + filename);
        }
        if (::read(fd, res.getBytes(), size) == -1) {
            close(fd);
            throw MedException("Failed to read from " + filename);
        }
        close(fd);
    } else if ((size_t)nread != size) {
        throw MedException("Partial read from process memory");
    }

    return res;
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
