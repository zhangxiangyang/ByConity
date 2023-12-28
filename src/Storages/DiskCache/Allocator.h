#pragma once

#include <mutex>

#include <Storages/DiskCache/Region.h>
#include <Storages/DiskCache/RegionManager.h>
#include <Storages/DiskCache/Types.h>
#include <common/types.h>

namespace DB::HybridCache
{
// Allocate a  particular size from a region.
class RegionAllocator
{
public:
    explicit RegionAllocator(UInt16 priority_) : priority{priority_} { }

    RegionAllocator(const RegionAllocator &) = delete;
    RegionAllocator & operator=(const RegionAllocator &) = delete;
    RegionAllocator(RegionAllocator && other) noexcept : priority{other.priority}, rid{other.rid} { }

    // Sets new region to allocate from.
    void setAllocationRegion(RegionId rid_);

    // Returns the current allocation region unique ID.
    RegionId getAllocationRegion() const { return rid; }

    // Resets allocator to the initial state.
    void reset();

    // Returns the priority this region allocator is associated with.
    UInt16 getPriority() const { return priority; }

    // Retruns the mutex lock.
    std::mutex & getLock() const { return mutex; }

private:
    const UInt16 priority{};

    RegionId rid;

    mutable std::mutex mutex;
};

// Size class or stack allocator.
class Allocator
{
public:
    explicit Allocator(RegionManager & region_manager_, UInt16 num_priorities);

    Allocator(const Allocator &) = delete;
    Allocator & operator=(const Allocator &) = delete;

    // Allocates and opens for writing.
    std::tuple<RegionDescriptor, UInt32, RelAddress> allocate(UInt32 size, UInt16 priority);

    // Closes the region.
    void close(RegionDescriptor && desc);

    // Resets the region to the initial state.
    void reset();

    // Flushes any regions with in-memory buffers to device.
    void flush();

private:
    Poco::Logger * log = &Poco::Logger::get("BlockCacheAllocator");
    
    using LockGuard = std::lock_guard<std::mutex>;

    void flushAndReleaseRegionFromRALocked(RegionAllocator & ra, bool flushAsync);

    std::tuple<RegionDescriptor, UInt32, RelAddress> allocateWith(RegionAllocator & ra, UInt32 size);

    RegionManager & region_manager;

    std::vector<RegionAllocator> allocators;
};
}
