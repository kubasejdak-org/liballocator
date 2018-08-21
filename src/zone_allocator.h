/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2017-2018, Kuba Sejdak <kuba.sejdak@gmail.com>
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// 1. Redistributions of source code must retain the above copyright notice, this
///    list of conditions and the following disclaimer.
///
/// 2. Redistributions in binary form must reproduce the above copyright notice,
///    this list of conditions and the following disclaimer in the documentation
///    and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
/// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
/// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
/// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
/// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
/// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///
/////////////////////////////////////////////////////////////////////////////////////

#ifndef ZONE_ALLOCATOR_H
#define ZONE_ALLOCATOR_H

#include "zone.h"
#include "utils.h"

#include <array>
#include <cstddef>

namespace Memory {

class PageAllocator;

/// @class ZoneAllocator
/// @brief Represents the zone allocator.
class ZoneAllocator {
public:
    /// @brief Default constructor.
    ZoneAllocator();

    /// @brief Initializes the ZoneAllocator with the given PageAllocator and page size.
    /// @param[in] pageAllocator        PageAllocator to be used in ZoneAllocator.
    /// @param[in] pageSize             Size of the physical page.
    /// @return True on success, false otherwise.
    [[nodiscard]] bool init(PageAllocator* pageAllocator, std::size_t pageSize);

    /// @brief Clears the ZoneAllocator internal state.
    void clear();

    /// @brief Allocates the memory chunk of at least given size.
    /// @param[in] size                 Size of the demanded memory chunk.
    /// @return Pointer to the allocated memory chunk on success, nullptr otherwise.
    [[nodiscard]] void* allocate(std::size_t size);

    /// @brief Releases the given memory chunk.
    /// @param[in] ptr                  Pointer to the memory chunk to be released.
    /// @note This function accepts nullptr input.
    void release(void* ptr);

private:
    /// @brief Allocates memory chunk from the given zone.
    /// @param[in] zone                 Zone from which chunk should be allocated.
    /// @return Allocated memory chunk.
    /// @note Template parameter is used here to cast the returned value the given type.
    template <typename T>
    T* allocateChunk(Zone* zone);

    
    /// @brief Deallocates memory chunk to the given zone.
    /// @param[in] chunk                Chunk to be deallocated.
    /// @return True on success, false otherwise.
    /// @note Template parameter is used here to accept any input without casting.
    template <typename T>
    bool deallocateChunk(T* chunk);

    /// @brief Returns size rounded up to the closest chunk size.
    /// @param[in] size                 Size to be rounded up.
    /// @return Closest chunk size.
    std::size_t chunkSize(std::size_t size);

    /// @brief Returns an index of the zone with the given chunk size.
    /// @param[in] chunkSize            Chunk size to be used in calculations.
    /// @return Index of the zone in the array of all known zones.
    std::size_t zoneIdx(std::size_t chunkSize);

    /// @brief Returns the Zone from the given array index, that has at least one free chunk.
    /// @param[in] idx                  Index from which Zone should be taken.
    /// @return Pointer to the Zone on success, nullptr otherwise.
    Zone* getFreeZone(std::size_t idx);

    /// @brief Checks if there is the minimal required number of free chunks in the zone at given array index.
    /// @param[in] idx                  Index to be checked.
    /// @return True if new Zone with the given index should be allocated, false otherwise.
    bool shouldAllocateZone(std::size_t idx);

    /// @brief Allocates new Zone with the chunks of given size.
    /// @param[in] chunkSize            Size of the chunks in the allocated zone.
    /// @return Pointer to the allocated Zone on success, nullptr otherwise.
    Zone* allocateZone(std::size_t chunkSize);

    /// @brief Initializes given zone.
    /// @param[in,out] zone             Zone to be initialized.
    /// @param[in] chunkSize            Size of the chunks in this zone.
    /// @return True on success, false otherwise.
    bool initZone(Zone* zone, std::size_t chunkSize);

    /// @brief Clears the given zone.
    /// @param[in,out] zone             Zone to be cleared.
    void clearZone(Zone* zone);

    /// @brief Adds the given zone to the array of known zones.
    /// @param[in,out] zone             Zone to be added.
    void addZone(Zone* zone);

    /// @brief Removes the given zone from the array of known zones.
    /// @param[in,out] zone             Zone to be removed.
    void removeZone(Zone* zone);

    /// @brief Finds the Zone that given chunk belong to.
    /// @param[in,out] chunk            Chunk for which zone should be found.
    /// @return Zone that given chunk belong to if found, nullptr otherwise.
    Zone* findZone(Chunk* chunk);

private:
    static constexpr std::size_t MINIMAL_ALLOC_SIZE = 16;   ///< Miniamal size of chunk, that can be allocated.
    static constexpr std::size_t MAX_ZONE_IDX = 8;          ///< Maximal supported index of the zone.

private:
    /// @class ZoneInfo
    /// @brief Represents the meta-data of the zone.
    struct ZoneInfo {
        Zone* head = nullptr;                               ///< Head of the zones with the given index.
        std::size_t freeChunksCount = 0;                    ///< Total number of free chunks in zones with the given index.
    };

    PageAllocator* m_pageAllocator;                         ///< PageAllocator to be used as source of new pages.
    std::size_t m_pageSize;                                 ///< Size of the page on this platform.
    std::size_t m_zoneDescChunkSize;                        ///< Size of the chunks that are used to store zone descriptors.
    std::size_t m_zoneDescIdx;                              ///< Index of the zones, from which zone descriptors are allocated.
    Zone m_initialZone;                                     ///< Initial static zone.
    std::array<ZoneInfo, MAX_ZONE_IDX> m_zones;             ///< Array of all zones known in the ZoneAllocator.
};

} // namespace Memory

#endif
