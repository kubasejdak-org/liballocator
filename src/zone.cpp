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

#include "zone.h"

#include "chunk.h"
#include "page.h"
#include "utils.h"

#include <cassert>

namespace Memory {

static_assert(Zone::isNaturallyAligned(), "class Zone is not naturally aligned");

void Zone::init(Page* page, std::size_t pageSize, std::size_t chunkSize)
{
    assert(page);
    assert(pageSize);
    assert(chunkSize);

    clear();

    m_page = page;
    m_chunkSize = chunkSize;
    m_chunksCount = pageSize / chunkSize;
    m_freeChunksCount = m_chunksCount;

    auto* chunk = reinterpret_cast<Chunk*>(page->address());
    for (std::size_t i = 0; i < m_chunksCount; ++i, chunk = utils_movePtr(chunk, m_chunkSize)) {
        chunk->init();
        chunk->addToList(&m_freeChunks);
    }
}

void Zone::clear()
{
    m_next = nullptr;
    m_prev = nullptr;
    m_page = nullptr;
    m_chunkSize = 0;
    m_chunksCount = 0;
    m_freeChunksCount = 0;
    m_freeChunks = nullptr;
}

void Zone::addToList(Zone** list)
{
    assert(list);
    assert(!m_next);
    assert(!m_prev);

    if (*list) {
        m_next = *list;
        m_next->m_prev = this;
    }

    *list = this;
}

void Zone::removeFromList(Zone** list)
{
    assert(list);
    assert(this == *list || m_next || m_prev);

    if (m_next)
        m_next->m_prev = m_prev;

    if (m_prev)
        m_prev->m_next = m_next;

    if (*list == this)
        *list = m_next;

    m_next = nullptr;
    m_prev = nullptr;
}

Zone* Zone::next()
{
    return m_next;
}

Page* Zone::page()
{
    return m_page;
}

std::size_t Zone::chunkSize()
{
    return m_chunkSize;
}

std::size_t Zone::chunksCount()
{
    return m_chunksCount;
}

std::size_t Zone::freeChunksCount()
{
    return m_freeChunksCount;
}

Chunk* Zone::takeChunk()
{
    assert(m_freeChunksCount);

    auto* chunk = m_freeChunks;
    chunk->removeFromList(&m_freeChunks);
    --m_freeChunksCount;

    return chunk;
}

void Zone::giveChunk(Chunk* chunk)
{
    assert(chunk);

    chunk->addToList(&m_freeChunks);
    ++m_freeChunksCount;
}

bool Zone::isValidChunk(Chunk* chunk)
{
    auto* it = reinterpret_cast<Chunk*>(m_page->address());
    for (std::size_t i = 0; i < m_chunksCount; ++i, it = utils_movePtr(it, m_chunkSize)) {
        if (it == chunk)
            return true;
    }

    return false;
}

} // namespace Memory
