#ifndef WINDOWS_RESERVED_VIRTUAL_MEMORY_HPP
#define WINDOWS_RESERVED_VIRTUAL_MEMORY_HPP
#include <cstddef>
#include <memory>

namespace GraphGenerator::Windows
{
    struct ReservedVirtualMemory
    {
        void* base = nullptr;
        std::uintptr_t currentEnd = 0;
        std::uintptr_t limit = 0;
        std::uint32_t pageSize = 1;
        std::uint32_t allocationGranularity = 1;

        ReservedVirtualMemory() = default;
        ~ReservedVirtualMemory();
        ReservedVirtualMemory(ReservedVirtualMemory&& other) noexcept;
        ReservedVirtualMemory& operator=(ReservedVirtualMemory&&) noexcept;
        void reserve(std::size_t bytes);
        void commit(std::size_t bytes);
    };
}

#endif // WINDOWS_RESERVED_VIRTUAL_MEMORY_HPP