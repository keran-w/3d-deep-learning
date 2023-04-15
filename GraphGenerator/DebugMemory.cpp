#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <PsApi.h>
#include <memory>
#include <mutex>
#include <unordered_map>

struct DebugMemory
{
    std::atomic_bool initialized = false;
    std::recursive_mutex mutex;
    std::unordered_map<void*, std::size_t> pointers = std::unordered_map<void*, std::size_t>::unordered_map();
    bool track = false;
    struct MemoryInfo
    {
        std::int64_t currentMemory = 0;
        std::int64_t maxMemory = 0;
        std::int64_t currentPointers = 0;
        std::int64_t maxPointers = 0;
        std::int64_t totalAllocatedMemory = 0;
        std::int64_t totalAllocatedPointers = 0;
        std::int64_t maxAllocatedMemory = 0;
        std::int64_t minAllocatedMemory = std::numeric_limits<std::int64_t>::max();
        std::int64_t systemMemoryUsage = 0;
    } info;


    DebugMemory()
    {
        initialized = true;
        track = true;
    }

    DebugMemory(DebugMemory&&) = delete;

    ~DebugMemory()
    {
        track = false;
        for (auto [p, s] : pointers)
        {
            std::free(p);
        }
        pointers.clear();
    }

    void trackPointer(void* p, std::size_t n)
    {
        if (not initialized)
        {
            return;
        }
        auto const lock = std::scoped_lock{ mutex };
        if (not track)
        {
            return;
        }
        track = false;
        pointers[p] += n;
        info.totalAllocatedMemory += n;
        ++info.totalAllocatedPointers;
        info.minAllocatedMemory = std::min<std::int64_t>(info.minAllocatedMemory, n);
        info.maxAllocatedMemory = std::max<std::int64_t>(info.maxAllocatedMemory, n);
        info.currentMemory += pointers[p];
        info.currentPointers = pointers.size();
        info.maxMemory = std::max(info.maxMemory, info.currentMemory);
        info.maxPointers = std::max<std::int64_t>(info.maxPointers, pointers.size());
        track = true;
    }

    void untrackPointer(void* p)
    {
        if (not initialized)
        {
            return;
        }
        auto const lock = std::scoped_lock{ mutex };
        if (not track)
        {
            return;
        }
        track = false;
        info.currentMemory -= pointers[p];
        pointers.erase(p);
        info.currentPointers = pointers.size();
        track = true;
    }
};

DebugMemory debug;
extern "C"
{
    DebugMemory::MemoryInfo getMemoryInfo()
    {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        debug.info.systemMemoryUsage = pmc.PrivateUsage;
        return debug.info;
    }
}

// #define RC_DEBUG_MEMORY
#ifdef RC_DEBUG_MEMORY
// replacement of a minimal set of functions:
void* operator new(std::size_t sz) // no inline, required by [replacement.functions]/3
{
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success

    if (void* ptr = std::malloc(sz))
    {
        debug.trackPointer(ptr, sz);
        return ptr;
    }

    throw std::bad_alloc{}; // required by [new.delete.single]/3
}
void operator delete(void* ptr) noexcept
{
    debug.untrackPointer(ptr);
    std::free(ptr);
}


void* operator new(std::size_t sz, std::align_val_t align) // no inline, required by [replacement.functions]/3
{
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success

    if (void* ptr = _aligned_malloc(sz, static_cast<std::size_t>(align)))
    {
        debug.trackPointer(ptr, sz);
        return ptr;
    }

    throw std::bad_alloc{};
}
void operator delete(void* ptr, std::align_val_t align) noexcept
{
    debug.untrackPointer(ptr);
    _aligned_free(ptr);
}
#endif // RC_DEBUG_MEMORY
#endif // _WIN32