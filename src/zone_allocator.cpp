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

#include "page_allocator.h"

#include <cassert>

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
        // TODO: implement.
        return nullptr;
    }

    std::size_t allocSize = chunkSize(size);
    std::size_t idx = zoneIdx(allocSize);

    if (!m_zones[idx]) {
        // TODO: implement.
    }

    // Zone* zone = m_zones[idx];

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

    // Algorithm to find next highest power of 2.
    --chunkSize;
    chunkSize |= chunkSize >> 1;
    chunkSize |= chunkSize >> 2;
    chunkSize |= chunkSize >> 4;
    chunkSize |= chunkSize >> 8;
    chunkSize |= chunkSize >> 16;
    ++chunkSize;

    return chunkSize;
}

std::size_t ZoneAllocator::zoneIdx(std::size_t chunkSize __attribute__((unused)))
{
    // TODO: implement.
    return 0;
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

} // namespace Memory
