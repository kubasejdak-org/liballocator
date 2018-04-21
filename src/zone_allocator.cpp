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

#include "zone_allocator.h"

#include "chunk.h"
#include "page_allocator.h"
#include "utils.h"

#include <cassert>
#include <cmath>

namespace Memory {

ZoneAllocator::ZoneAllocator()
{
    clear();
}

bool ZoneAllocator::init(PageAllocator* pageAllocator, std::size_t pageSize)
{
    clear();

    m_pageAllocator = pageAllocator;
    m_pageSize = pageSize;

    auto* page = m_pageAllocator->allocate(1);
    if (!page)
        return false;

    m_initialZone.init(page, m_pageSize, chunkSize(sizeof(Zone)));
    addZone(&m_initialZone);
    return true;
}

void ZoneAllocator::clear()
{
    m_pageAllocator = nullptr;
    m_pageSize = 0;
    m_zones.fill(nullptr);
    m_initialZone.clear();
}

void* ZoneAllocator::allocate(std::size_t size)
{
    if (!size)
        return nullptr;

    if (size >= m_pageSize) {
        auto pageCount = static_cast<std::size_t>(std::ceil(double(size) / double(m_pageSize)));
        auto* page = m_pageAllocator->allocate(pageCount);
        return reinterpret_cast<void*>(page->address());
    }

    std::size_t allocSize = chunkSize(size);
    std::size_t idx = zoneIdx(allocSize);

    for (auto* zone = m_zones[idx]; zone != nullptr; zone = zone->next()) {
        if (!zone->freeChunksCount())
            continue;

        return reinterpret_cast<void*>(zone->takeChunk());
    }

    // TODO: alloc new zone.

    return nullptr;
}

void ZoneAllocator::release(void* ptr)
{
    if (!ptr)
        return;
}

std::size_t ZoneAllocator::chunkSize(std::size_t size)
{
    std::size_t chunkSize = (size < MINIMAL_ALLOC_SIZE) ? MINIMAL_ALLOC_SIZE : size;
    return utils_roundPower2(chunkSize);
}

std::size_t ZoneAllocator::zoneIdx(std::size_t chunkSize)
{
    return static_cast<std::size_t>(std::floor(std::log2(chunkSize)) - 4);
}

void ZoneAllocator::addZone(Zone* zone)
{
    assert(zone);

    auto idx = zoneIdx(zone->chunkSize());
    zone->addToList(&m_zones[idx]);
}

void ZoneAllocator::removeZone(Zone* zone)
{
    assert(zone);
    assert(zone != &m_initialZone);

    auto idx = zoneIdx(zone->chunkSize());
    zone->removeFromList(&m_zones[idx]);
}

Zone* ZoneAllocator::findZone(Chunk* chunk)
{
    assert(chunk);

    // TODO: implement.
    return nullptr;
}

} // namespace Memory
