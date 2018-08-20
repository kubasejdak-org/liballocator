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

#ifndef ZONE_H
#define ZONE_H

#include "list_node.h"

#include <cstddef>

namespace Memory {

class Page;

/// @class Chunk
/// @brief Represents a memory chunk. Chunks are part of the zone.
/// @note Each chunk has the size, which is a power of 2.
class Chunk : public ListNode<Chunk> {};

/// @class Zone
/// @brief Represents a memory zone. Each zone consists of the memory chunks of equal size.
class Zone : public ListNode<Zone> {
public:
    /// @brief Default constructor.
    Zone() = default;

    /// @brief Copy constructor.
    /// @param[in] other        Zone to be used in initialization.
    /// @note This constructor is deleted, because zones should be initialized only in-place.
    Zone(const Zone& other) = delete;

    /// @brief Move constructor.
    /// @param[in] other        Page to be used in initialization.
    /// @note This constructor is deleted, because pages should be initialized only in-place.
    Zone(Zone&& other) = delete;

    /// @brief Initializes the zone. It is used as a replacement for the constructor.
    /// @param[in] page         page to be associated with this zone.
    /// @param[in] pageSize     Size of the associated page.
    /// @param[in] chunkSize    Size of the chunk to be used within this zone.
    void init(Page* page, std::size_t pageSize, std::size_t chunkSize);

    /// @brief Clears the internal state of the zone.
    void clear();

    /// @brief Returns the page, that this zone is bound to.
    /// @return Page, that this zone is associated with.
    Page* page();

    /// @brief Returns size of the chunks, that create this zone.
    /// @return Size of the chunks created from this zone.
    std::size_t chunkSize();

    /// @brief Returns total count of the chunks, that are part of this zone.
    /// @return Number of chunks, that create this zone.
    std::size_t chunksCount();

    /// @brief Returns number of the non-allocated chunks in this zone.
    /// @return Number of chunks, that are not allocated in this zone.
    std::size_t freeChunksCount();

    /// @brief Allocates the chunk from this zone and returns it.
    /// @return Allocated chunk.
    /// @note This function updates the 'free' counter.
    Chunk* takeChunk();

    /// @brief Releases the given chunk.
    /// @param[in] chunk        Chunk to be released.
    /// @note This function updates the 'free' counter.
    void giveChunk(Chunk* chunk);

    /// @brief Checks if given chunk is part of the current zone and if it is valid.
    /// @param[in] chunk        Chunk to be checked.
    /// @return True if given chunk is valid, false otherwise.
    bool isValidChunk(Chunk* chunk);

    /// @brief Checks if the Zone class is naturally aligned.
    /// @return True if Zone class is naturally aligned, false otherwise.
    /// @note Natural alignment of a class means, that its size is equal to the sum of all its data members.
    static constexpr bool isNaturallyAligned()
    {
        constexpr std::size_t requiredSize = sizeof(ListNode<Zone>) // Inherited fields
                                           + sizeof(Page*)          // m_page
                                           + sizeof(std::size_t)    // m_chunkSize
                                           + sizeof(std::size_t)    // m_chunksCount
                                           + sizeof(std::size_t)    // m_freeChunksCount
                                           + sizeof(Chunk*);        // m_freeChunks
        return (requiredSize == sizeof(Zone));
    }

private:
    Page* m_page;                       ///< Page, that is associated with this zone.
    std::size_t m_chunkSize;            ///< Size of the chunks, that are part of this zone.
    std::size_t m_chunksCount;          ///< Number of chunks in this zone.
    std::size_t m_freeChunksCount;      ///< Number of free chunks in this zone.
    Chunk* m_freeChunks;                ///< List of free chunks in this zone.
};

} // namespace Memory

#endif
