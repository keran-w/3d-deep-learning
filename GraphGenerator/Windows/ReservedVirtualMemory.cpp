#include "ReservedVirtualMemory.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cassert>
#include <stdexcept>
#include <system_error>

namespace GraphGenerator::Windows
{
    ReservedVirtualMemory::~ReservedVirtualMemory()
    {
        if (base == nullptr)
        {
            return;
        }
        BOOL result = VirtualFree(base, 0, MEM_RELEASE);
        assert(result != FALSE);
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
        SYSTEM_INFO systemInfo = { 0 };
        GetSystemInfo(&systemInfo);
        allocationGranularity = systemInfo.dwAllocationGranularity;
        pageSize = systemInfo.dwPageSize;
        // round bytes to allocation granularity
        bytes = (((bytes - 1) / allocationGranularity) + 1) * allocationGranularity;
        // reserve memory
        base = VirtualAlloc(nullptr, bytes, MEM_RESERVE, PAGE_READWRITE);
        if (base == nullptr)
        {
            int error = static_cast<int>(GetLastError());
            throw std::system_error{ error, std::system_category(), "VirtualAlloc reserve" };
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
        if (VirtualAlloc(address, bytes, MEM_COMMIT, PAGE_READWRITE) != address)
        {
            int error = static_cast<int>(GetLastError());
            throw std::system_error{ error, std::system_category(), "VirtualAlloc commit" };
        }
        currentEnd += bytes;
    }
}
#endif // _WIN32