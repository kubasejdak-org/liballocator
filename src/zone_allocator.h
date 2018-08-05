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
class Chunk;

class ZoneAllocator {
public:
    ZoneAllocator();

    [[nodiscard]] bool init(PageAllocator* pageAllocator, std::size_t pageSize);
    void clear();

    [[nodiscard]] void* allocate(std::size_t size);
    void release(void* ptr);

private:
    template <typename T>
    T* allocateChunk(Zone* zone);
    template <typename T>
    bool deallocateChunk(T* chunk);

    std::size_t chunkSize(std::size_t size);
    std::size_t zoneIdx(std::size_t chunkSize);
    Zone* getFreeZone(std::size_t idx);
    bool shouldAllocateZone(std::size_t idx);
    Zone* allocateZone(std::size_t chunkSize);
    bool initZone(Zone* zone, std::size_t chunkSize);
    void clearZone(Zone* zone);
    void addZone(Zone* zone);
    void removeZone(Zone* zone);
    Zone* findZone(Chunk* chunk);

private:
    static constexpr std::size_t MINIMAL_ALLOC_SIZE = 16;
    static constexpr std::size_t MAX_ZONE_IDX = 8;

private:
    struct ZoneInfo {
        Zone* head = nullptr;
        std::size_t freeChunksCount = 0;
    };

    PageAllocator* m_pageAllocator;
    std::size_t m_pageSize;
    std::size_t m_zoneDescChunkSize;
    std::size_t m_zoneDescIdx;
    Zone m_initialZone;
    std::array<ZoneInfo, MAX_ZONE_IDX> m_zones;
};

} // namespace Memory

#endif
