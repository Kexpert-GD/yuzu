// Copyright 2019 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <span>
#include <utility>
#include <vector>
#include "common/common_types.h"
#include "video_core/vulkan_common/vulkan_wrapper.h"

namespace Vulkan {

class Device;
class MemoryMap;
class MemoryAllocation;

/// Hints and requirements for the backing memory type of a commit
enum class MemoryUsage {
    DeviceLocal, ///< Hints device local usages, fastest memory type to read and write from the GPU
    Upload,      ///< Requires a host visible memory type optimized for CPU to GPU uploads
    Download,    ///< Requires a host visible memory type optimized for GPU to CPU readbacks
};

/// Ownership handle of a memory commitment.
/// Points to a subregion of a memory allocation.
class MemoryCommit {
public:
    explicit MemoryCommit() noexcept = default;
    explicit MemoryCommit(MemoryAllocation* allocation_, VkDeviceMemory memory_, u64 begin_,
                          u64 end_) noexcept;
    ~MemoryCommit();

    MemoryCommit& operator=(MemoryCommit&&) noexcept;
    MemoryCommit(MemoryCommit&&) noexcept;

    MemoryCommit& operator=(const MemoryCommit&) = delete;
    MemoryCommit(const MemoryCommit&) = delete;

    /// Returns a host visible memory map.
    /// It will map the backing allocation if it hasn't been mapped before.
    std::span<u8> Map();

    /// Returns the Vulkan memory handler.
    VkDeviceMemory Memory() const {
        return memory;
    }

    /// Returns the start position of the commit relative to the allocation.
    VkDeviceSize Offset() const {
        return static_cast<VkDeviceSize>(begin);
    }

private:
    void Release();

    MemoryAllocation* allocation{}; ///< Pointer to the large memory allocation.
    VkDeviceMemory memory{};        ///< Vulkan device memory handler.
    u64 begin{};                    ///< Beginning offset in bytes to where the commit exists.
    u64 end{};                      ///< Offset in bytes where the commit ends.
    std::span<u8> span;             ///< Host visible memory span. Empty if not queried before.
};

/// Memory allocator container.
/// Allocates and releases memory allocations on demand.
class MemoryAllocator {
public:
    explicit MemoryAllocator(const Device& device_);
    ~MemoryAllocator();

    MemoryAllocator& operator=(const MemoryAllocator&) = delete;
    MemoryAllocator(const MemoryAllocator&) = delete;

    /**
     * Commits a memory with the specified requirements.
     *
     * @param requirements Requirements returned from a Vulkan call.
     * @param usage        Indicates how the memory will be used.
     *
     * @returns A memory commit.
     */
    MemoryCommit Commit(const VkMemoryRequirements& requirements, MemoryUsage usage);

    /// Commits memory required by the buffer and binds it.
    MemoryCommit Commit(const vk::Buffer& buffer, MemoryUsage usage);

    /// Commits memory required by the image and binds it.
    MemoryCommit Commit(const vk::Image& image, MemoryUsage usage);

private:
    /// Allocates a chunk of memory.
    void AllocMemory(VkMemoryPropertyFlags flags, u32 type_mask, u64 size);

    /// Tries to allocate a memory commit.
    std::optional<MemoryCommit> TryCommit(const VkMemoryRequirements& requirements,
                                          VkMemoryPropertyFlags flags);

    /// Returns the fastest compatible memory property flags from a wanted usage.
    VkMemoryPropertyFlags MemoryPropertyFlags(u32 type_mask, MemoryUsage usage) const;

    /// Returns the fastest compatible memory property flags from the wanted flags.
    VkMemoryPropertyFlags MemoryPropertyFlags(u32 type_mask, VkMemoryPropertyFlags flags) const;

    /// Returns index to the fastest memory type compatible with the passed requirements.
    std::optional<u32> FindType(VkMemoryPropertyFlags flags, u32 type_mask) const;

    const Device& device;                                       ///< Device handle.
    const VkPhysicalDeviceMemoryProperties properties;          ///< Physical device properties.
    std::vector<std::unique_ptr<MemoryAllocation>> allocations; ///< Current allocations.
};

/// Returns true when a memory usage is guaranteed to be host visible.
bool IsHostVisible(MemoryUsage usage) noexcept;

} // namespace Vulkan
