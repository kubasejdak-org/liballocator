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

#include "zone_allocator.h"

#include "page_allocator.h"
#include "utils.h"

#include <cassert>
#include <cmath>

namespace memory {

ZoneAllocator::ZoneAllocator() noexcept
{
    clear();
}

bool ZoneAllocator::init(PageAllocator* pageAllocator, std::size_t pageSize)
{
    clear();

    m_pageAllocator = pageAllocator;
    m_pageSize = pageSize;
    m_zoneDescChunkSize = chunkSize(sizeof(Zone));
    m_zoneDescIdx = zoneIdx(m_zoneDescChunkSize);
    return initZone(&m_initialZone, m_zoneDescChunkSize);
}

void ZoneAllocator::clear()
{
    m_pageAllocator = nullptr;
    m_pageSize = 0;
    m_zoneDescChunkSize = 0;
    m_zoneDescIdx = 0;
    m_initialZone.clear();
    m_zones.fill({});
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

    Zone* zone = shouldAllocateZone(idx) ? allocateZone(allocSize) : getFreeZone(idx);
    return allocateChunk<void>(zone);
}

void ZoneAllocator::release(void* ptr)
{
    if (!ptr)
        return;

    if (deallocateChunk(ptr))
        return;

    auto* pages = reinterpret_cast<Page*>(ptr);
    m_pageAllocator->release(pages);
}

template <typename T>
T* ZoneAllocator::allocateChunk(Zone* zone)
{
    std::size_t idx = zoneIdx(zone->chunkSize());
    m_zones[idx].freeChunksCount--;
    return reinterpret_cast<T*>(zone->takeChunk());
}

template <typename T>
bool ZoneAllocator::deallocateChunk(T* chunk)
{
    auto* zoneChunk = reinterpret_cast<Chunk*>(chunk);
    auto* zone = findZone(zoneChunk);
    if (!zone)
        return false;

    if (!zone->isValidChunk(zoneChunk))
        return false;

    std::size_t idx = zoneIdx(zone->chunkSize());
    m_zones[idx].freeChunksCount++;
    zone->giveChunk(zoneChunk);

    if (zone->chunksCount() == zone->freeChunksCount()) {
        removeZone(zone);
        clearZone(zone);
        return deallocateChunk(zone);
    }

    return true;
}

std::size_t ZoneAllocator::chunkSize(std::size_t size)
{
    std::size_t chunkSize = (size < MINIMAL_ALLOC_SIZE) ? MINIMAL_ALLOC_SIZE : size;
    return utils::roundPower2(chunkSize);
}

std::size_t ZoneAllocator::zoneIdx(std::size_t chunkSize)
{
    return static_cast<std::size_t>(std::floor(std::log2(chunkSize)) - 4);
}

Zone* ZoneAllocator::getFreeZone(std::size_t idx)
{
    for (auto* zone = m_zones[idx].head; zone != nullptr; zone = zone->next()) {
        if (!zone->freeChunksCount())
            continue;

        return zone;
    }

    return nullptr;
}

bool ZoneAllocator::shouldAllocateZone(std::size_t idx)
{
    std::size_t triggerCount = (idx == m_zoneDescIdx) ? 1 : 0;
    return (m_zones[idx].freeChunksCount == triggerCount);
}

Zone* ZoneAllocator::allocateZone(std::size_t chunkSize)
{
    if (chunkSize != m_zoneDescChunkSize && shouldAllocateZone(m_zoneDescIdx))
        allocateZone(m_zoneDescChunkSize);

    auto* zone = getFreeZone(m_zoneDescIdx);
    auto* newZone = allocateChunk<Zone>(zone);
    if (!initZone(newZone, chunkSize))
        return nullptr;

    addZone(newZone);
    return newZone;
}

bool ZoneAllocator::initZone(Zone* zone, std::size_t chunkSize)
{
    assert(zone);

    if (auto* page = m_pageAllocator->allocate(1)) {
        zone->init(page, m_pageSize, chunkSize);
        return true;
    }

    return false;
}

void ZoneAllocator::clearZone(Zone* zone)
{
    assert(zone);

    m_pageAllocator->release(zone->page());
    zone->clear();
}

void ZoneAllocator::addZone(Zone* zone)
{
    assert(zone);

    auto idx = zoneIdx(zone->chunkSize());
    zone->addToList(&m_zones[idx].head);
    m_zones[idx].freeChunksCount += zone->freeChunksCount();
}

void ZoneAllocator::removeZone(Zone* zone)
{
    assert(zone);
    assert(zone != &m_initialZone);

    auto idx = zoneIdx(zone->chunkSize());
    zone->removeFromList(&m_zones[idx].head);
    m_zones[idx].freeChunksCount -= zone->freeChunksCount();
}

Zone* ZoneAllocator::findZone(Chunk* chunk)
{
    assert(chunk);

    auto chunkAddr = reinterpret_cast<std::uintptr_t>(chunk);
    auto* page = reinterpret_cast<Page*>(chunkAddr & ~(m_pageSize - 1));

    for (auto& zoneInfo : m_zones) {
        for (auto* zone = zoneInfo.head; zone != nullptr; zone = zone->next()) {
            if (zone->page() == page)
                return zone;
        }
    }

    return nullptr;
}

} // namespace memory
