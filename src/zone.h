////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017-2018, Kuba Sejdak <kuba.sejdak@gmail.com>
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
////////////////////////////////////////////////////////////////////////////////////

#ifndef ZONE_H
#define ZONE_H

#include <cstddef>

namespace Memory {

class Page;
class Chunk;

class Zone {
public:
    Zone() = default;
    Zone(const Zone& other) = delete;
    Zone(Zone&& other) = delete;

    void init(Page* page, std::size_t pageSize, std::size_t chunkSize);
    void clear();
    void addToList(Zone** list);
    void removeFromList(Zone** list);
    Zone* next();
    Page* page();
    std::size_t chunkSize();
    std::size_t chunksCount();
    std::size_t freeChunksCount();

    Chunk* takeChunk();
    void giveChunk(Chunk* chunk);
    bool isValidChunk(Chunk* chunk);

    static constexpr bool isNaturallyAligned()
    {
        constexpr std::size_t requiredSize = sizeof(Zone*)          // m_next
                                           + sizeof(Zone*)          // m_prev
                                           + sizeof(Page*)          // m_page
                                           + sizeof(std::size_t)    // m_chunkSize
                                           + sizeof(std::size_t)    // m_chunksCount
                                           + sizeof(std::size_t)    // m_freeChunksCount
                                           + sizeof(Chunk*);        // m_freeChunks
        return (requiredSize == sizeof(Zone));
    }

private:
    Zone* m_next;
    Zone* m_prev;
    Page* m_page;
    std::size_t m_chunkSize;
    std::size_t m_chunksCount;
    std::size_t m_freeChunksCount;
    Chunk* m_freeChunks;
};

} // namespace Memory

#endif
