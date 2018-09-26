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

#include <catch2/catch.hpp>

#include "test_utils.h"

#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <map>
#include <random>

// Make access to private members for testing.
// clang-format off
#define private     public
// clang-format on

#include <page_allocator.h>
#include <utils.h>
#include <zone_allocator.h>

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace memory;

TEST_CASE("ZoneAllocator is properly cleared", "[zone_allocator]")
{
    ZoneAllocator zoneAllocator;
    std::memset(reinterpret_cast<void*>(&zoneAllocator), 0x5a, sizeof(ZoneAllocator));

    zoneAllocator.clear();
    REQUIRE(zoneAllocator.m_pageAllocator == nullptr);
    REQUIRE(zoneAllocator.m_pageSize == 0);
    REQUIRE(zoneAllocator.m_zoneDescChunkSize == 0);
    REQUIRE(zoneAllocator.m_zoneDescIdx == 0);
    REQUIRE(zoneAllocator.m_initialZone.m_next == nullptr);
    REQUIRE(zoneAllocator.m_initialZone.m_prev == nullptr);
    REQUIRE(zoneAllocator.m_initialZone.m_page == nullptr);
    REQUIRE(zoneAllocator.m_initialZone.m_chunkSize == 0);
    REQUIRE(zoneAllocator.m_initialZone.m_chunksCount == 0);
    REQUIRE(zoneAllocator.m_initialZone.m_freeChunksCount == 0);
    REQUIRE(zoneAllocator.m_initialZone.m_freeChunks == nullptr);
    for (const auto& zone : zoneAllocator.m_zones) {
        REQUIRE(zone.head == nullptr);
        REQUIRE(zone.freeChunksCount == 0);
    }

    auto stats = zoneAllocator.getStats();
    REQUIRE(stats.usedMemorySize == 0);
    REQUIRE(stats.reservedMemorySize == 0);
    REQUIRE(stats.freeMemorySize == 0);
    REQUIRE(stats.allocatedMemorySize == 0);
}

TEST_CASE("ZoneAllocator is properly initialized", "[zone_allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;

    SECTION("Free pages are available")
    {
        REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));
        REQUIRE(zoneAllocator.m_pageAllocator == &pageAllocator);
        REQUIRE(zoneAllocator.m_pageSize == pageSize);
        REQUIRE(zoneAllocator.m_zoneDescChunkSize == 64);
        REQUIRE(zoneAllocator.m_zoneDescIdx == 2);

        REQUIRE(zoneAllocator.m_initialZone.m_next == nullptr);
        REQUIRE(zoneAllocator.m_initialZone.m_prev == nullptr);
        REQUIRE(zoneAllocator.m_initialZone.m_chunkSize == zoneAllocator.m_zoneDescChunkSize);
        REQUIRE(zoneAllocator.m_initialZone.m_chunksCount == (pageSize / zoneAllocator.m_zoneDescChunkSize));
        REQUIRE(zoneAllocator.m_initialZone.m_freeChunksCount == (pageSize / zoneAllocator.m_zoneDescChunkSize));
        std::uintptr_t lastChunkAddr = zoneAllocator.m_initialZone.page()->address() + pageSize - zoneAllocator.m_zoneDescChunkSize;
        REQUIRE(zoneAllocator.m_initialZone.m_freeChunks == reinterpret_cast<Chunk*>(lastChunkAddr));

        auto* chunk = reinterpret_cast<Chunk*>(zoneAllocator.m_initialZone.page()->address());
        for (std::size_t i = 0; i < zoneAllocator.m_initialZone.chunksCount(); ++i) {
            REQUIRE(std::uintptr_t(chunk) == zoneAllocator.m_initialZone.page()->address() + i * zoneAllocator.m_zoneDescChunkSize);
            chunk = chunk->m_prev;
        }

        for (const auto& zone : zoneAllocator.m_zones) {
            if (zone.head == &zoneAllocator.m_initialZone) {
                REQUIRE(zone.freeChunksCount == (pageSize / zoneAllocator.m_zoneDescChunkSize));
                continue;
            }

            REQUIRE(zone.head == nullptr);
            REQUIRE(zone.freeChunksCount == 0);
        }

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == pageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == pageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }

    SECTION("Free pages are unavailable")
    {
        REQUIRE(pageAllocator.allocate(pageAllocator.m_freePagesCount));
        REQUIRE(!zoneAllocator.init(&pageAllocator, pageSize));

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == 0);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == 0);
        REQUIRE(stats.allocatedMemorySize == 0);
    }
}

TEST_CASE("ZoneAllocator stats are properly initialized", "[zone_allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));

    auto stats = zoneAllocator.getStats();
    REQUIRE(stats.usedMemorySize == pageSize);
    REQUIRE(stats.reservedMemorySize == 0);
    REQUIRE(stats.freeMemorySize == pageSize);
    REQUIRE(stats.allocatedMemorySize == 0);
}

TEST_CASE("Chunk size is properly calculated", "[zone_allocator]")
{
    ZoneAllocator zoneAllocator;
    std::size_t size = 0;
    std::size_t roundedSize = 0;

    SECTION("Size is smaller than the minimal alloc size")
    {
        size = ZoneAllocator::MINIMAL_ALLOC_SIZE / 2;
        roundedSize = zoneAllocator.chunkSize(size);
        REQUIRE(roundedSize == ZoneAllocator::MINIMAL_ALLOC_SIZE);
    }

    SECTION("Size is equal to the minimal alloc size")
    {
        size = ZoneAllocator::MINIMAL_ALLOC_SIZE;
        roundedSize = zoneAllocator.chunkSize(size);
        REQUIRE(roundedSize == ZoneAllocator::MINIMAL_ALLOC_SIZE);
    }

    SECTION("Size is greater than the minimal alloc size")
    {
        size = ZoneAllocator::MINIMAL_ALLOC_SIZE * 2;
        roundedSize = zoneAllocator.chunkSize(size);
        REQUIRE(roundedSize > ZoneAllocator::MINIMAL_ALLOC_SIZE);
    }

    REQUIRE(roundedSize >= size);
    REQUIRE(std::pow(2.0, std::log2(double(roundedSize))) == double(roundedSize));
}

TEST_CASE("Zone index is properly calculated", "[zone_allocator]")
{
    ZoneAllocator zoneAllocator;
    std::map<std::size_t, std::pair<std::size_t, size_t>> idxRange = {
        {0, {16, 31}},
        {1, {32, 63}},
        {2, {64, 127}},
        {3, {128, 255}},
        {4, {256, 511}},
        {5, {512, 1023}},
        {6, {1024, 2047}},
        {7, {2048, 4095}}};

    for (std::size_t i = idxRange[0].first; i < idxRange[7].second; ++i) {
        auto idx = zoneAllocator.zoneIdx(i);
        REQUIRE(i >= idxRange[idx].first);
        REQUIRE(i <= idxRange[idx].second);
    }
}

TEST_CASE("Zone is properly initialized by the zone allocator", "[zone_allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));

    std::size_t chunkSize = 128;
    Zone zone;
    zone.clear();

    SECTION("There is at least one free page")
    {
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        REQUIRE(zoneAllocator.initZone(&zone, chunkSize));
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount - 1);
        REQUIRE(zone.m_next == nullptr);
        REQUIRE(zone.m_prev == nullptr);
        REQUIRE(zone.m_chunkSize == chunkSize);
        REQUIRE(zone.m_chunksCount == (pageSize / chunkSize));
        REQUIRE(zone.m_freeChunksCount == (pageSize / chunkSize));
        std::uintptr_t lastChunkAddr = zone.page()->address() + pageSize - chunkSize;
        REQUIRE(zone.m_freeChunks == reinterpret_cast<Chunk*>(lastChunkAddr));
    }

    SECTION("There are no free pages")
    {
        REQUIRE(pageAllocator.allocate(pageAllocator.m_freePagesCount));
        REQUIRE(!zoneAllocator.initZone(&zone, chunkSize));
    }
}

TEST_CASE("Zone is properly cleared by the zone allocator", "[zone_allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));

    std::size_t chunkSize = 128;
    Zone zone;
    zone.clear();
    REQUIRE(zoneAllocator.initZone(&zone, chunkSize));

    std::size_t freePagesCount = pageAllocator.m_freePagesCount;
    zoneAllocator.clearZone(&zone);
    REQUIRE(pageAllocator.m_freePagesCount == freePagesCount + 1);
}

TEST_CASE("Zone is properly added", "[zone_allocator]")
{
    ZoneAllocator zoneAllocator;
    zoneAllocator.clear();

    Zone zone;
    zone.clear();

    std::size_t idx = 0;

    SECTION("Zone is added at index 0")
    {
        zone.m_chunkSize = 16;
        idx = 0;
    }

    SECTION("Zone is added at index 1")
    {
        zone.m_chunkSize = 32;
        idx = 1;
    }

    SECTION("Zone is added at index 3")
    {
        zone.m_chunkSize = 128;
        idx = 3;
    }

    zoneAllocator.addZone(&zone);

    bool found = false;
    for (auto* it = zoneAllocator.m_zones[idx].head; it != nullptr; it = it->next()) {
        if (it == &zone) {
            found = true;
            break;
        }
    }

    REQUIRE(found);
}

TEST_CASE("Zone is properly removed", "[zone_allocator]")
{
    ZoneAllocator zoneAllocator;
    zoneAllocator.clear();

    Zone zone;
    zone.clear();

    std::size_t idx = 0;

    SECTION("Zone is removed from index 0")
    {
        zone.m_chunkSize = 16;
        idx = 0;
    }

    SECTION("Zone is removed from index 1")
    {
        zone.m_chunkSize = 32;
        idx = 1;
    }

    SECTION("Zone is removed from index 3")
    {
        zone.m_chunkSize = 128;
        idx = 3;
    }

    zoneAllocator.addZone(&zone);
    zoneAllocator.removeZone(&zone);

    bool found = false;
    for (auto* it = zoneAllocator.m_zones[idx].head; it != nullptr; it = it->next()) {
        if (it == &zone) {
            found = true;
            break;
        }
    }

    REQUIRE(!found);
}

TEST_CASE("Zone allocator properly finds the zones", "[zone_allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));

    SECTION("Existing chunk from index 0")
    {
        Zone zone;
        zone.clear();
        zoneAllocator.initZone(&zone, 16);
        zoneAllocator.addZone(&zone);
        auto* chunk = zone.takeChunk();
        REQUIRE(zoneAllocator.findZone(chunk) == &zone);
    }

    SECTION("Existing chunk from index 3")
    {
        Zone zone;
        zone.clear();
        zoneAllocator.initZone(&zone, 128);
        zoneAllocator.addZone(&zone);
        auto* chunk = zone.takeChunk();
        REQUIRE(zoneAllocator.findZone(chunk) == &zone);
    }

    SECTION("Non-existing chunk")
    {
        Page* page = pageAllocator.allocate(1);
        auto* chunk = reinterpret_cast<Chunk*>(page->address());
        REQUIRE(!zoneAllocator.findZone(chunk));
    }
}

TEST_CASE("Zone allocator properly checks if a zone should be allocated", "[zone_allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));

    SECTION("Allocating from zoneDescIdx index, trigger not satisfied")
    {
        std::size_t idx = zoneAllocator.m_zoneDescIdx;
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == 4);
        REQUIRE(!zoneAllocator.shouldAllocateZone(idx));
    }

    SECTION("Allocating from zoneDescIdx index, trigger satisfied")
    {
        std::size_t idx = zoneAllocator.m_zoneDescIdx;
        std::size_t takeCount = zoneAllocator.m_zones[idx].freeChunksCount - 1;
        for (std::size_t i = 0; i < takeCount; ++i) {
            zoneAllocator.m_zones[idx].head->takeChunk();
            --zoneAllocator.m_zones[idx].freeChunksCount;
        }

        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == 1);
        REQUIRE(zoneAllocator.shouldAllocateZone(idx));
    }

    SECTION("Allocating from other index than zoneDescIdx, trigger not satisfied")
    {
        std::size_t chunkSize = 16;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);

        Zone zone;
        zone.clear();
        zoneAllocator.initZone(&zone, chunkSize);
        zoneAllocator.addZone(&zone);

        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == (pageSize / chunkSize));
        REQUIRE(!zoneAllocator.shouldAllocateZone(idx));
    }

    SECTION("Allocating from other index than zoneDescIdx, trigger satisfied")
    {
        std::size_t chunkSize = 16;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);

        Zone zone;
        zone.clear();
        zoneAllocator.initZone(&zone, chunkSize);
        zoneAllocator.addZone(&zone);

        std::size_t takeCount = zoneAllocator.m_zones[idx].freeChunksCount;
        for (std::size_t i = 0; i < takeCount; ++i) {
            zoneAllocator.m_zones[idx].head->takeChunk();
            --zoneAllocator.m_zones[idx].freeChunksCount;
        }
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == 0);
        REQUIRE(zoneAllocator.shouldAllocateZone(idx));
    }
}

TEST_CASE("Zone allocator properly finds free zones", "[zone_allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));

    SECTION("No free zones are available at all")
    {
        std::size_t idx = zoneAllocator.m_zoneDescIdx;
        std::size_t takeCount = zoneAllocator.m_zones[idx].freeChunksCount;
        for (std::size_t i = 0; i < takeCount; ++i) {
            zoneAllocator.m_zones[idx].head->takeChunk();
            --zoneAllocator.m_zones[idx].freeChunksCount;
        }

        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == 0);
        REQUIRE(!zoneAllocator.getFreeZone(idx));
    }

    SECTION("One free zone is available with, but with different index")
    {
        std::size_t idx = zoneAllocator.m_zoneDescIdx + 1;
        REQUIRE(!zoneAllocator.getFreeZone(idx));
    }

    SECTION("One free zone with demanded index is available")
    {
        std::size_t idx = zoneAllocator.m_zoneDescIdx;
        REQUIRE(zoneAllocator.getFreeZone(idx) == &zoneAllocator.m_initialZone);
    }

    SECTION("Two zones with the same index available, only second has free chunks")
    {
        Zone zone;
        zone.clear();
        std::size_t chunkSize = zoneAllocator.m_zoneDescChunkSize;
        zoneAllocator.initZone(&zone, chunkSize);
        zoneAllocator.addZone(&zone);

        std::size_t idx = zoneAllocator.m_zoneDescIdx;
        std::size_t takeCount = zone.m_freeChunksCount;
        for (std::size_t i = 0; i < takeCount; ++i) {
            zoneAllocator.m_zones[idx].head->takeChunk();
            --zoneAllocator.m_zones[idx].freeChunksCount;
        }

        REQUIRE(zoneAllocator.getFreeZone(idx) == &zoneAllocator.m_initialZone);
    }
}

TEST_CASE("Zone allocator properly allocates chunks", "[zone_allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));

    SECTION("Allocate chunk from zone with index 0")
    {
        std::size_t chunkSize = 16;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);

        Zone zone;
        zone.clear();
        zoneAllocator.initZone(&zone, chunkSize);
        zoneAllocator.addZone(&zone);

        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(&zone);
        REQUIRE(chunk);
        REQUIRE(zone.isValidChunk(chunk));
        REQUIRE(zone.m_freeChunksCount == zone.m_chunksCount - 1);
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount - 1);

        auto* it = zone.m_freeChunks;
        for (std::size_t i = 0; i < zone.freeChunksCount(); ++i, it = it->next())
            REQUIRE(it != chunk);
    }

    SECTION("Allocate chunk from zone with index 3")
    {
        std::size_t chunkSize = 128;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);

        Zone zone;
        zone.clear();
        zoneAllocator.initZone(&zone, chunkSize);
        zoneAllocator.addZone(&zone);

        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(&zone);
        REQUIRE(chunk);
        REQUIRE(zone.isValidChunk(chunk));
        REQUIRE(zone.m_freeChunksCount == zone.m_chunksCount - 1);
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount - 1);

        auto* it = zone.m_freeChunks;
        for (std::size_t i = 0; i < zone.freeChunksCount(); ++i, it = it->next())
            REQUIRE(it != chunk);
    }
}

TEST_CASE("Zone allocator properly deallocates chunks", "[zone_allocator]")
{
    constexpr std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));

    SECTION("Deallocate chunk from zone with index 0")
    {
        std::size_t chunkSize = 16;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        auto* zone = zoneAllocator.allocateChunk<Zone>(&zoneAllocator.m_initialZone);
        zone->clear();
        zoneAllocator.initZone(zone, chunkSize);
        zoneAllocator.addZone(zone);

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(zone);

        REQUIRE(zoneAllocator.deallocateChunk(chunk));
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Deallocate chunk from zone with index 3")
    {
        std::size_t chunkSize = 256;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        auto* zone = zoneAllocator.allocateChunk<Zone>(&zoneAllocator.m_initialZone);
        zone->clear();
        zoneAllocator.initZone(zone, chunkSize);
        zoneAllocator.addZone(zone);

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(zone);

        REQUIRE(zoneAllocator.deallocateChunk(chunk));
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Deallocate address from the middle of the valid chunk")
    {
        std::size_t chunkSize = 16;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);

        auto* zone = zoneAllocator.allocateChunk<Zone>(&zoneAllocator.m_initialZone);
        zone->clear();
        zoneAllocator.initZone(zone, chunkSize);
        zoneAllocator.addZone(zone);

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(zone);
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        REQUIRE(!zoneAllocator.deallocateChunk(utils::movePtr(chunk, chunkSize / 2)));
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Deallocate invalid chunk")
    {
        Page* page = pageAllocator.allocate(1);
        auto* chunk = reinterpret_cast<Chunk*>(page->address());
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        REQUIRE(!zoneAllocator.deallocateChunk(chunk));
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Deallocate the penultimate chunk from zone with index 0")
    {
        std::size_t chunkSize = 16;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);

        auto* zone = zoneAllocator.allocateChunk<Zone>(&zoneAllocator.m_initialZone);
        zone->clear();
        zoneAllocator.initZone(zone, chunkSize);
        zoneAllocator.addZone(zone);

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(zone);
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        zoneAllocator.allocateChunk<Chunk>(zone);

        REQUIRE(zoneAllocator.deallocateChunk(chunk));
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Deallocate whole zone with index 0")
    {
        constexpr std::size_t chunkSize = 16;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        auto* zone = zoneAllocator.allocateChunk<Zone>(&zoneAllocator.m_initialZone);
        zone->clear();
        zoneAllocator.initZone(zone, chunkSize);
        zoneAllocator.addZone(zone);

        std::array<Chunk*, (pageSize / chunkSize)> chunks{};
        for (Chunk*& chunk : chunks)
            chunk = zoneAllocator.allocateChunk<Chunk>(zone);

        for (Chunk* chunk : chunks)
            REQUIRE(zoneAllocator.deallocateChunk<Chunk>(chunk));

        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Deallocate last chunk from zone with index m_zoneDescIdx, but not from initial zone")
    {
        std::size_t chunkSize = 64;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        REQUIRE(idx == zoneAllocator.m_zoneDescIdx);

        auto* zone = zoneAllocator.allocateChunk<Zone>(&zoneAllocator.m_initialZone);
        zone->clear();
        zoneAllocator.initZone(zone, chunkSize);
        zoneAllocator.addZone(zone);

        std::size_t freeChunksCount = zoneAllocator.m_initialZone.freeChunksCount();
        for (std::size_t i = 0; i < freeChunksCount; ++i)
            zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(zone);

        REQUIRE(zoneAllocator.deallocateChunk(chunk));
        REQUIRE(zoneAllocator.m_initialZone.freeChunksCount() == 1);
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == zoneAllocator.m_initialZone.freeChunksCount());
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }
}

TEST_CASE("Zone allocator properly allocates zones", "[zone_allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));

    SECTION("Allocate zone with index 0, no need to preallocate a zone for zone descriptors")
    {
        std::size_t chunkSize = 16;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);

        auto* zone = zoneAllocator.allocateZone(chunkSize);
        REQUIRE(zone);
        REQUIRE(zone->chunkSize() == chunkSize);

        bool found = false;
        for (auto* it = zoneAllocator.m_zones[idx].head; it != nullptr; it = it->next()) {
            if (it == zone) {
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }

    SECTION("Allocate zone with index 0, need to preallocate a zone for zone descriptors")
    {
        std::size_t chunkSize = 16;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);

        zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);
        zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);
        zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);
        auto* zone = zoneAllocator.allocateZone(chunkSize);
        REQUIRE(zone);
        REQUIRE(zone->chunkSize() == chunkSize);

        bool found = false;
        for (auto* it = zoneAllocator.m_zones[idx].head; it != nullptr; it = it->next()) {
            if (it == zone) {
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }

    SECTION("Allocate zone with index 3, no need to preallocate a zone for zone descriptors")
    {
        std::size_t chunkSize = 128;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);

        auto* zone = zoneAllocator.allocateZone(chunkSize);
        REQUIRE(zone);
        REQUIRE(zone->chunkSize() == chunkSize);

        bool found = false;
        for (auto* it = zoneAllocator.m_zones[idx].head; it != nullptr; it = it->next()) {
            if (it == zone) {
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }

    SECTION("Allocate zone with index m_zoneDescIdx, no need to preallocate a zone for zone descriptors")
    {
        std::size_t chunkSize = 64;
        std::size_t idx = zoneAllocator.zoneIdx(chunkSize);

        auto* zone = zoneAllocator.allocateZone(chunkSize);
        REQUIRE(zone);
        REQUIRE(zone->chunkSize() == chunkSize);

        bool found = false;
        for (auto* it = zoneAllocator.m_zones[idx].head; it != nullptr; it = it->next()) {
            if (it == zone) {
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }

    SECTION("Allocate zone with index 0, need to preallocate a zone for zone descriptors, no pages left")
    {
        std::size_t chunkSize = 16;
        zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);
        zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);
        zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);
        REQUIRE(pageAllocator.allocate(pageAllocator.m_freePagesCount));
        REQUIRE(!zoneAllocator.allocateZone(chunkSize));
    }

    SECTION("No pages left to initialize the new zone")
    {
        std::size_t chunkSize = 16;
        REQUIRE(pageAllocator.allocate(pageAllocator.m_freePagesCount));
        REQUIRE(!zoneAllocator.allocateZone(chunkSize));
    }
}

TEST_CASE("Zone allocator properly allocates user memory", "[zone_allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));

    SECTION("Allocate 0 bytes")
    {
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        REQUIRE(!zoneAllocator.allocate(0));
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == pageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == pageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }

    SECTION("Allocate size equal to 3 pages")
    {
        std::size_t allocPagesCount = 3;
        std::size_t allocSize = allocPagesCount * pageSize;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        REQUIRE(zoneAllocator.allocate(allocSize));
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount - allocPagesCount);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == pageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == pageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }

    SECTION("Allocate 6 bytes")
    {
        std::size_t allocSize = 6;
        std::size_t idx = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;
        auto* ptr = zoneAllocator.allocate(allocSize);
        REQUIRE(ptr);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount - 1);
        std::size_t chunkSize = zoneAllocator.m_zones[idx].head->chunkSize();
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount + chunkSize - 1);

        bool valid = false;
        for (auto* zone = zoneAllocator.m_zones[idx].head; zone != nullptr; zone = zone->next()) {
            if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                valid = true;
                break;
            }
        }
        REQUIRE(valid);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == 2 * pageSize);
        REQUIRE(stats.reservedMemorySize == zoneAllocator.m_zoneDescChunkSize);
        REQUIRE(stats.freeMemorySize == (2 * pageSize - stats.reservedMemorySize - zoneAllocator.chunkSize(allocSize)));
        REQUIRE(stats.allocatedMemorySize == zoneAllocator.chunkSize(allocSize));
    }

    SECTION("Allocate 16 bytes")
    {
        std::size_t allocSize = 16;
        std::size_t idx = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;
        auto* ptr = zoneAllocator.allocate(allocSize);
        REQUIRE(ptr);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount - 1);
        std::size_t chunkSize = zoneAllocator.m_zones[idx].head->chunkSize();
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount + chunkSize - 1);

        bool valid = false;
        for (auto* zone = zoneAllocator.m_zones[idx].head; zone != nullptr; zone = zone->next()) {
            if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                valid = true;
                break;
            }
        }
        REQUIRE(valid);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == 2 * pageSize);
        REQUIRE(stats.reservedMemorySize == zoneAllocator.m_zoneDescChunkSize);
        REQUIRE(stats.freeMemorySize == (2 * pageSize - stats.reservedMemorySize - zoneAllocator.chunkSize(allocSize)));
        REQUIRE(stats.allocatedMemorySize == zoneAllocator.chunkSize(allocSize));
    }

    SECTION("Allocate 64 bytes")
    {
        std::size_t allocSize = 64;
        std::size_t idx = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;
        auto* ptr = zoneAllocator.allocate(allocSize);
        REQUIRE(ptr);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount - 1);

        bool valid = false;
        for (auto* zone = zoneAllocator.m_zones[idx].head; zone != nullptr; zone = zone->next()) {
            if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                valid = true;
                break;
            }
        }
        REQUIRE(valid);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == pageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == (pageSize - stats.reservedMemorySize - zoneAllocator.chunkSize(allocSize)));
        REQUIRE(stats.allocatedMemorySize == zoneAllocator.chunkSize(allocSize));
    }

    SECTION("Allocate 64 bytes 6 times")
    {
        std::size_t allocSize = 64;
        std::size_t idx = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        std::array<void*, 6> ptrs{};
        for (void*& ptr : ptrs) {
            ptr = zoneAllocator.allocate(allocSize);
            REQUIRE(ptr);
        }

        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount - 1);
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == 1);

        for (void* ptr : ptrs) {
            bool valid = false;
            for (auto* zone = zoneAllocator.m_zones[idx].head; zone != nullptr; zone = zone->next()) {
                if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                    valid = true;
                    break;
                }
            }
            REQUIRE(valid);
        }

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == 2 * pageSize);
        REQUIRE(stats.reservedMemorySize == zoneAllocator.m_zoneDescChunkSize);
        REQUIRE(stats.freeMemorySize == (2 * pageSize - stats.reservedMemorySize - 6 * zoneAllocator.chunkSize(allocSize)));
        REQUIRE(stats.allocatedMemorySize == 6 * zoneAllocator.chunkSize(allocSize));
    }

    SECTION("Allocate 115 bytes 4 times")
    {
        std::size_t allocSize = 115;
        std::size_t idx = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        std::array<void*, 4> ptrs{};
        for (void*& ptr : ptrs) {
            ptr = zoneAllocator.allocate(allocSize);
            REQUIRE(ptr);
        }

        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount - 2);
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == 0);

        for (void* ptr : ptrs) {
            bool valid = false;
            for (auto* zone = zoneAllocator.m_zones[idx].head; zone != nullptr; zone = zone->next()) {
                if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                    valid = true;
                    break;
                }
            }
            REQUIRE(valid);
        }

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == 3 * pageSize);
        REQUIRE(stats.reservedMemorySize == 2 * zoneAllocator.m_zoneDescChunkSize);
        REQUIRE(stats.freeMemorySize == (3 * pageSize - stats.reservedMemorySize - 4 * zoneAllocator.chunkSize(allocSize)));
        REQUIRE(stats.allocatedMemorySize == 4 * zoneAllocator.chunkSize(allocSize));
    }

    SECTION("Allocate 64 bytes 3 times, then 128 bytes 3 times")
    {
        // Allocate 64 bytes 3 times.
        std::size_t allocSize1 = 64;
        std::size_t idx1 = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize1));
        std::size_t freePagesCount1 = pageAllocator.m_freePagesCount;

        std::array<void*, 3> ptrs1{};
        for (void*& ptr : ptrs1) {
            ptr = zoneAllocator.allocate(allocSize1);
            REQUIRE(ptr);
        }

        for (void* ptr : ptrs1) {
            bool valid = false;
            for (auto* zone = zoneAllocator.m_zones[idx1].head; zone != nullptr; zone = zone->next()) {
                if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                    valid = true;
                    break;
                }
            }
            REQUIRE(valid);
        }

        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount1);
        REQUIRE(zoneAllocator.m_zones[zoneAllocator.m_zoneDescIdx].freeChunksCount == 1);

        // Allocate 128 bytes 3 times.
        std::size_t allocSize2 = 128;
        std::size_t idx2 = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize2));
        std::size_t freePagesCount2 = pageAllocator.m_freePagesCount;

        std::array<void*, 3> ptrs2{};
        for (void*& ptr : ptrs2) {
            ptr = zoneAllocator.allocate(allocSize2);
            REQUIRE(ptr);
        }

        for (void* ptr : ptrs2) {
            bool valid = false;
            for (auto* zone = zoneAllocator.m_zones[idx2].head; zone != nullptr; zone = zone->next()) {
                if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                    valid = true;
                    break;
                }
            }
            REQUIRE(valid);
        }

        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount2 - 3);
        REQUIRE(zoneAllocator.m_zones[zoneAllocator.m_zoneDescIdx].freeChunksCount == 2);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == 4 * pageSize);
        REQUIRE(stats.reservedMemorySize == 3 * zoneAllocator.m_zoneDescChunkSize);
        REQUIRE(stats.freeMemorySize == (4 * pageSize - stats.reservedMemorySize - 3 * (zoneAllocator.chunkSize(allocSize1) + zoneAllocator.chunkSize(allocSize2))));
        REQUIRE(stats.allocatedMemorySize == 3 * (zoneAllocator.chunkSize(allocSize1) + zoneAllocator.chunkSize(allocSize2)));
    }

    SECTION("Allocate 4 pages, no pages are available")
    {
        std::size_t allocSize = 4 * pageSize;
        REQUIRE(pageAllocator.allocate(pageAllocator.m_freePagesCount));
        REQUIRE(!zoneAllocator.allocate(allocSize));

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == pageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == pageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }

    SECTION("Allocate 128 bytes, no pages are available")
    {
        std::size_t allocSize = 128;
        REQUIRE(pageAllocator.allocate(pageAllocator.m_freePagesCount));
        REQUIRE(!zoneAllocator.allocate(allocSize));

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == pageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == pageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }
}

TEST_CASE("Zone allocator properly releases user memory", "[zone_allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));

    SECTION("Release nullptr")
    {
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        zoneAllocator.release(nullptr);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Release invalid pointer")
    {
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        zoneAllocator.release(reinterpret_cast<void*>(0xdeadbeef));
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Release memory with size equal to 3 pages")
    {
        std::size_t allocPagesCount = 3;
        std::size_t allocSize = allocPagesCount * pageSize;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        auto* ptr = zoneAllocator.allocate(allocSize);
        zoneAllocator.release(ptr);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Release 6 bytes")
    {
        std::size_t allocSize = 6;
        std::size_t idx = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;
        auto* ptr = zoneAllocator.allocate(allocSize);
        zoneAllocator.release(ptr);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount);
    }

    SECTION("Release 16 bytes")
    {
        std::size_t allocSize = 16;
        std::size_t idx = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;
        auto* ptr = zoneAllocator.allocate(allocSize);
        zoneAllocator.release(ptr);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount);
    }

    SECTION("Release 64 bytes")
    {
        std::size_t allocSize = 64;
        std::size_t idx = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;
        auto* ptr = zoneAllocator.allocate(allocSize);
        zoneAllocator.release(ptr);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount);
    }

    SECTION("Release 64 bytes 6 times")
    {
        std::size_t allocSize = 64;
        std::size_t idx = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;

        std::array<void*, 6> ptrs{};
        for (void*& ptr : ptrs)
            ptr = zoneAllocator.allocate(allocSize);

        for (void* ptr : ptrs)
            zoneAllocator.release(ptr);

        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount);
    }

    SECTION("Release 115 bytes 4 times")
    {
        std::size_t allocSize = 115;
        std::size_t idx = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones[idx].freeChunksCount;

        std::array<void*, 4> ptrs{};
        for (void*& ptr : ptrs)
            ptr = zoneAllocator.allocate(allocSize);

        for (void* ptr : ptrs)
            zoneAllocator.release(ptr);

        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones[idx].freeChunksCount == freeChunksCount);
    }

    SECTION("Release 64 bytes 3 times, then 128 bytes 3 times")
    {
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        // Allocate 64 bytes 3 times.
        std::size_t allocSize1 = 64;
        std::size_t idx1 = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize1));
        std::size_t freeChunksCount1 = zoneAllocator.m_zones[idx1].freeChunksCount;

        std::array<void*, 3> ptrs1{};
        for (void*& ptr : ptrs1)
            ptr = zoneAllocator.allocate(allocSize1);

        // Allocate 128 bytes 3 times.
        std::size_t allocSize2 = 128;
        std::size_t idx2 = zoneAllocator.zoneIdx(zoneAllocator.chunkSize(allocSize2));
        std::size_t freeChunksCount2 = zoneAllocator.m_zones[idx2].freeChunksCount;

        std::array<void*, 3> ptrs2{};
        for (void*& ptr : ptrs2)
            ptr = zoneAllocator.allocate(allocSize2);

        // Release 64 bytes 3 times.
        for (void* ptr : ptrs1)
            zoneAllocator.release(ptr);

        // Release 128 bytes 3 times.
        for (void* ptr : ptrs2)
            zoneAllocator.release(ptr);

        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones[idx1].freeChunksCount == freeChunksCount1);
        REQUIRE(zoneAllocator.m_zones[idx2].freeChunksCount == freeChunksCount2);
    }

    auto stats = zoneAllocator.getStats();
    REQUIRE(stats.usedMemorySize == pageSize);
    REQUIRE(stats.reservedMemorySize == 0);
    REQUIRE(stats.freeMemorySize == pageSize);
    REQUIRE(stats.allocatedMemorySize == 0);
}

TEST_CASE("ZoneAllocator integration tests (long-term)", "[zone_allocator][integration][.]")
{
    using namespace std::chrono_literals;
    constexpr auto testDuration = 30min;
    constexpr int allocationsCount = 100;

    std::size_t pageSize = 256;
    std::size_t pagesCount = 256;
    PageAllocator pageAllocator;

    auto size = pageSize * pagesCount;
    auto memory = test::alignedAlloc(pageSize, size);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    };
    // clang-format on

    REQUIRE(pageAllocator.init(regions, pageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, pageSize));

    auto freePagesCount = pageAllocator.m_freePagesCount;
    auto maxAllocSize = 2 * pageSize;

    // Initialize random number generator.
    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());
    std::uniform_int_distribution<std::size_t> distribution(0, maxAllocSize);

    std::array<void*, allocationsCount> ptrs{};

    for (auto start = test::currentTime(); !test::timeElapsed(start, testDuration);) {
        ptrs.fill(nullptr);

        // Allocate memory.
        for (auto*& ptr : ptrs) {
            auto allocSize = distribution(randomGenerator);
            ptr = zoneAllocator.allocate(allocSize);
        }

        // Release memory.
        for (auto* ptr : ptrs)
            zoneAllocator.release(ptr);

        auto* chunk = zoneAllocator.m_initialZone.m_freeChunks;
        std::size_t chunkCount = 0;
        while (chunk) {
            ++chunkCount;
            chunk = chunk->next();
        }
        REQUIRE(chunkCount == zoneAllocator.m_initialZone.freeChunksCount());
        REQUIRE(chunkCount == zoneAllocator.m_initialZone.chunksCount());
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);

        for (const auto& zone : zoneAllocator.m_zones) {
            if (zone.head == &zoneAllocator.m_initialZone) {
                REQUIRE(zone.freeChunksCount == (pageSize / zoneAllocator.m_zoneDescChunkSize));
                continue;
            }

            REQUIRE(zone.head == nullptr);
            REQUIRE(zone.freeChunksCount == 0);
        }

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == pageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == pageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }
}
