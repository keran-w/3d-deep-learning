#include "ReservedVirtualMemory.hpp"
#ifndef _WIN32
#include <sys/mman.h>
#include <unistd.h>
#include <cassert>
#include <stdexcept>
#include <system_error>

namespace GraphGenerator::Unix
{
    ReservedVirtualMemory::~ReservedVirtualMemory()
    {
        if (base == nullptr)
        {
            return;
        }
        munmap(base, limit - reinterpret_cast<std::uintptr_t>(base));
    }

    ReservedVirtualMemory::ReservedVirtualMemory(ReservedVirtualMemory&& other) noexcept
    {
        *this = std::move(other);
    }

    ReservedVirtualMemory& ReservedVirtualMemory::operator=(ReservedVirtualMemory&& other) noexcept
    {
        base = std::exchange(other.base, base);
        currentEnd = std::exchange(other.currentEnd, currentEnd);
        limit = std::exchange(other.limit, limit);
        pageSize = std::exchange(other.pageSize, pageSize);
        allocationGranularity = std::exchange(other.allocationGranularity, allocationGranularity);
        return *this;
    }

    void ReservedVirtualMemory::reserve(std::size_t bytes)
    {
        if (base != nullptr)
        {
            throw std::logic_error{ "memory already reserved, ReservedVirtualMemory cannot be reused" };
        }
        auto systemPageSize = sysconf(_SC_PAGE_SIZE);
        if (systemPageSize == -1)
        {
            throw std::system_error{ errno, std::system_category(), "sysconf" };
        }
        if (systemPageSize > std::numeric_limits<decltype(pageSize)>::max())
        {
            throw std::range_error{ "page size too large" };
        }
        pageSize = static_cast<std::uint32_t>(systemPageSize);
        allocationGranularity = pageSize;
        // round bytes to allocation granularity
        bytes = (((bytes - 1) / allocationGranularity) + 1) * allocationGranularity;
        // reserve memory
        base = mmap(nullptr, bytes, PROT_NONE, MAP_PRIVATE bitor MAP_ANONYMOUS, -1, 0);
        if (base == nullptr)
        {
            throw std::system_error{ errno, std::system_category(), "mmap" };
        }
        currentEnd = reinterpret_cast<std::uintptr_t>(base);
        limit = currentEnd + bytes;
    }

    void ReservedVirtualMemory::commit(std::size_t bytes)
    {
        // round bytes to page size
        bytes = (((bytes - 1) / pageSize) + 1) * pageSize;
        if ((currentEnd >= limit) or ((limit - currentEnd) < bytes))
        {
            throw std::bad_alloc{};
        }
        void* address = reinterpret_cast<void*>(currentEnd);
        if (mprotect(address, bytes, PROT_READ | PROT_WRITE) == -1)
        {
            throw std::system_error{ errno, std::system_category(), "VirtualAlloc commit" };
        }
        currentEnd += bytes;
    }
}
#endif // _WIN32