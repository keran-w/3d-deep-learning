#ifndef WINDOWS_MONOTONIC_ALLOCATOR_HPP
#define WINDOWS_MONOTONIC_ALLOCATOR_HPP
#ifdef _WIN32
#include "ReservedVirtualMemory.hpp"
#else
#include "../Unix/ReservedVirtualMemory.hpp"
using GraphGenerator::Unix::ReservedVirtualMemory;
#endif // _WIN32
#include <stdexcept>
#include <string>

namespace GraphGenerator::Windows
{
    struct MonotonicAllocatorState
    {
        using OffsetType = std::uint32_t;

        ReservedVirtualMemory memory;
        std::uintptr_t allocatorBase = 0;
        std::uintptr_t currentOffset = 0;
        std::uintptr_t currentLimit = 0;
        std::size_t granularity = 16 * 1024 * 1024;

        void reserve(std::size_t bytes)
        {
            if (allocatorBase != 0)
            {
                throw std::logic_error{ "already reserved" };
            }
            memory.reserve(std::max(granularity, bytes));
            allocatorBase = memory.currentEnd;
            // prevent "null" difference, or null handle
            allocate<alignof(std::max_align_t)>(sizeof(std::max_align_t));
            if ((memory.limit - allocatorBase) >= std::numeric_limits<OffsetType>::max())
            {
                throw std::range_error{ "memory range too large: " + std::to_string(allocatorBase) + " -> " + std::to_string(memory.limit) };
            }
        }

        template<std::size_t alignment>
        void* allocate(std::size_t bytes)
        {
            if ((currentLimit - currentOffset) >= (bytes + alignment))
            {
                std::size_t test = alignment - 1;
                // align
                std::uintptr_t result = (currentOffset + test) / alignment * alignment;
                currentOffset = result + bytes;
                return reinterpret_cast<void*>(allocatorBase + result);
            }
            memory.commit(granularity);
            currentLimit = memory.currentEnd - allocatorBase;
            return allocate<alignment>(bytes);
        }

        void deallocate(void* p, std::size_t bytes)
        {
            // no-op
        }
    };

    template<typename T>
    struct MonotonicAllocator
    {
        using value_type = T;
        using handle = MonotonicAllocatorState::OffsetType;
        
        using StorageType = std::conditional_t<std::is_void_v<T>, std::byte, T>;
        struct FreeNode
        {
            FreeNode* next;
        };
        std::uintptr_t allocatorBase = 0;
        FreeNode* nextFree = nullptr;
        std::shared_ptr<MonotonicAllocatorState> state = std::make_shared<MonotonicAllocatorState>();

        MonotonicAllocator() noexcept {}
        MonotonicAllocator(std::size_t n)
        {
            if (n >= std::numeric_limits<handle>::max())
            {
                throw std::range_error{ "too large" };
            }
            state->reserve(n * sizeof(StorageType));
            allocatorBase = state->allocatorBase;
        }
        template<typename U>
        MonotonicAllocator(MonotonicAllocator<U> const& other) noexcept :
            allocatorBase{ other.allocatorBase },
            state{ other.state }
        {}
        MonotonicAllocator& operator=(MonotonicAllocator const& other) noexcept
        {
            allocatorBase = other.allocatorBase;
            state = other.state;
            return *this;
        }

        friend bool operator==(MonotonicAllocator const& a, MonotonicAllocator const& b) noexcept
        {
            return a.state == b.state;
        }
        friend bool operator!=(MonotonicAllocator const& a, MonotonicAllocator const& b) noexcept
        {
            return not (a == b);
        }

        template<typename U>
        MonotonicAllocator<U> clone() const
        {
            std::uintptr_t base = reinterpret_cast<std::uintptr_t>(state->memory.base);
             return { (state->memory.limit - base) / sizeof(value_type) };
        }

        T* allocate(std::size_t n)
        {
            std::size_t constexpr alignment = std::max({ alignof(StorageType), alignof(FreeNode), sizeof(StorageType) });
            if constexpr (sizeof(StorageType) < sizeof(FreeNode))
            {
                throw std::runtime_error{ "small type is not supported" };
            }

            if ((n != 1) or (nextFree == nullptr))
            {
                return static_cast<T*>(state->allocate<alignment>(n * sizeof(StorageType)));
            }
            FreeNode* result = nextFree;
            nextFree = nextFree->next;
            return reinterpret_cast<T*>(result);
        }

        void deallocate(T* p, std::size_t n)
        {
            if (n == 1)
            {
                if constexpr (sizeof(StorageType) < sizeof(FreeNode))
                {
                    throw std::runtime_error{ "small type is not supported" };
                }

                FreeNode* current = nextFree;
                nextFree = new (p) FreeNode{ .next = current };
            }
        }

        handle translate(T* p)
        {
            if (p != nullptr)
            {
                std::uintptr_t offset = reinterpret_cast<std::uintptr_t>(p) - allocatorBase;
                return static_cast<handle>(offset / sizeof(StorageType));
            }
            return {};
        }

        T* resolve(handle h)
        {
            if (h != handle{})
            {
                std::uintptr_t offset = static_cast<std::size_t>(h) * sizeof(StorageType);
                return reinterpret_cast<T*>(allocatorBase + offset);
            }
            return nullptr;
        }
    };
}
#endif // WINDOWS_MONOTONIC_ALLOCATOR_HPP