/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2017-2019, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <allocator/region.hpp>
#include <test_utils.hpp>
#include <utils.hpp>

#include <catch2/catch.hpp>

#include <array>
#include <cmath>
#include <cstring>
#include <map>

// Make access to private members for testing.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define private public

#include <page.hpp>
#include <page_allocator.hpp>
#include <zone_allocator.hpp>

namespace memory {

TEST_CASE("ZoneAllocator is properly cleared", "[unit][zone_allocator]")
{
    ZoneAllocator zoneAllocator;
    constexpr int cPattern = 0x5a;
    std::memset(reinterpret_cast<void*>(&zoneAllocator), cPattern, sizeof(ZoneAllocator));

    zoneAllocator.clear();

    auto stats = zoneAllocator.getStats();
    REQUIRE(stats.usedMemorySize == 0);
    REQUIRE(stats.reservedMemorySize == 0);
    REQUIRE(stats.freeMemorySize == 0);
    REQUIRE(stats.allocatedMemorySize == 0);
}

TEST_CASE("ZoneAllocator is properly initialized", "[unit][zone_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;

    SECTION("Free pages are available")
    {
        REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == cPageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == cPageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }

    SECTION("Free pages are unavailable")
    {
        REQUIRE(pageAllocator.allocate(pageAllocator.getStats().freePagesCount));
        REQUIRE(!zoneAllocator.init(&pageAllocator, cPageSize));

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == 0);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == 0);
        REQUIRE(stats.allocatedMemorySize == 0);
    }
}

TEST_CASE("ZoneAllocator stats are properly initialized", "[unit][zone_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    auto stats = zoneAllocator.getStats();
    REQUIRE(stats.usedMemorySize == cPageSize);
    REQUIRE(stats.reservedMemorySize == 0);
    REQUIRE(stats.freeMemorySize == cPageSize);
    REQUIRE(stats.allocatedMemorySize == 0);
}

TEST_CASE("Chunk size is properly calculated", "[unit][zone_allocator]")
{
    std::size_t size = 0;
    std::size_t roundedSize = 0;

    SECTION("Size is smaller than the minimal alloc size")
    {
        size = ZoneAllocator::cMinimalAllocSize / 2;
        roundedSize = detail::chunkSize(size);
        REQUIRE(roundedSize == ZoneAllocator::cMinimalAllocSize);
    }

    SECTION("Size is equal to the minimal alloc size")
    {
        size = ZoneAllocator::cMinimalAllocSize;
        roundedSize = detail::chunkSize(size);
        REQUIRE(roundedSize == ZoneAllocator::cMinimalAllocSize);
    }

    SECTION("Size is greater than the minimal alloc size")
    {
        size = ZoneAllocator::cMinimalAllocSize * 2;
        roundedSize = detail::chunkSize(size);
        REQUIRE(roundedSize > ZoneAllocator::cMinimalAllocSize);
    }

    REQUIRE(roundedSize >= size);
    REQUIRE(std::pow(2.0, std::log2(double(roundedSize))) == double(roundedSize)); // NOLINT
}

TEST_CASE("Zone index is properly calculated", "[unit][zone_allocator]")
{
    std::map<std::size_t, std::pair<std::size_t, size_t>> idxRange = {
        {0, {16, 31}},      // NOLINT
        {1, {32, 63}},      // NOLINT
        {2, {64, 127}},     // NOLINT
        {3, {128, 255}},    // NOLINT
        {4, {256, 511}},    // NOLINT
        {5, {512, 1023}},   // NOLINT
        {6, {1024, 2047}},  // NOLINT
        {7, {2048, 4095}}}; // NOLINT

    for (std::size_t i = idxRange[0].first; i < idxRange[idxRange.size() - 1].second; ++i) {
        auto idx = detail::zoneIdx(i);
        REQUIRE(i >= idxRange[idx].first);
        REQUIRE(i <= idxRange[idx].second);
    }
}

TEST_CASE("Zone is properly initialized by the zone allocator", "[unit][zone_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    constexpr std::size_t cChunkSize = 128;
    Zone zone;
    zone.clear();

    SECTION("There is at least one free page")
    {
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        REQUIRE(zoneAllocator.initZone(&zone, cChunkSize));
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount - 1);
        REQUIRE(zone.m_next == nullptr);
        REQUIRE(zone.m_prev == nullptr);
        REQUIRE(zone.m_chunkSize == cChunkSize);
        REQUIRE(zone.m_chunksCount == (cPageSize / cChunkSize));
        REQUIRE(zone.m_freeChunksCount == (cPageSize / cChunkSize));
        std::uintptr_t lastChunkAddr = zone.page()->address() + cPageSize - cChunkSize;
        REQUIRE(zone.m_freeChunks == reinterpret_cast<Chunk*>(lastChunkAddr));
    }

    SECTION("There are no free pages")
    {
        REQUIRE(pageAllocator.allocate(pageAllocator.m_freePagesCount));
        REQUIRE(!zoneAllocator.initZone(&zone, cChunkSize));
    }
}

TEST_CASE("Zone is properly cleared by the zone allocator", "[unit][zone_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    constexpr std::size_t cChunkSize = 128;
    Zone zone;
    zone.clear();
    REQUIRE(zoneAllocator.initZone(&zone, cChunkSize));

    std::size_t freePagesCount = pageAllocator.m_freePagesCount;
    zoneAllocator.clearZone(&zone);
    REQUIRE(pageAllocator.m_freePagesCount == freePagesCount + 1);
}

TEST_CASE("Zone is properly added", "[unit][zone_allocator]")
{
    ZoneAllocator zoneAllocator;
    zoneAllocator.clear();

    Zone zone;
    zone.clear();

    std::size_t idx = 0;

    SECTION("Zone is added at index 0")
    {
        zone.m_chunkSize = 16; // NOLINT
        idx = 0;
    }

    SECTION("Zone is added at index 1")
    {
        zone.m_chunkSize = 32; // NOLINT
        idx = 1;
    }

    SECTION("Zone is added at index 3")
    {
        zone.m_chunkSize = 128; // NOLINT
        idx = 3;
    }

    zoneAllocator.addZone(&zone);

    bool found = false;
    for (auto* it = zoneAllocator.m_zones.at(idx).head; it != nullptr; it = it->next()) {
        if (it == &zone) {
            found = true;
            break;
        }
    }

    REQUIRE(found);
}

TEST_CASE("Zone is properly removed", "[unit][zone_allocator]")
{
    ZoneAllocator zoneAllocator;
    zoneAllocator.clear();

    Zone zone;
    zone.clear();

    std::size_t idx = 0;

    SECTION("Zone is removed from index 0")
    {
        zone.m_chunkSize = 16; // NOLINT
        idx = 0;
    }

    SECTION("Zone is removed from index 1")
    {
        zone.m_chunkSize = 32; // NOLINT
        idx = 1;
    }

    SECTION("Zone is removed from index 3")
    {
        zone.m_chunkSize = 128; // NOLINT
        idx = 3;
    }

    zoneAllocator.addZone(&zone);
    zoneAllocator.removeZone(&zone);

    bool found = false;
    for (auto* it = zoneAllocator.m_zones.at(idx).head; it != nullptr; it = it->next()) {
        if (it == &zone) {
            found = true;
            break;
        }
    }

    REQUIRE(!found);
}

TEST_CASE("Zone allocator properly finds the zones", "[unit][zone_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    SECTION("Existing chunk from index 0")
    {
        Zone zone;
        zone.clear();
        constexpr std::size_t cChunkSize = 16;
        zoneAllocator.initZone(&zone, cChunkSize);
        zoneAllocator.addZone(&zone);
        auto* chunk = zone.takeChunk();
        REQUIRE(zoneAllocator.findZone(chunk) == &zone);
    }

    SECTION("Existing chunk from index 3")
    {
        Zone zone;
        zone.clear();
        constexpr std::size_t cChunkSize = 128;
        zoneAllocator.initZone(&zone, cChunkSize);
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

TEST_CASE("Zone allocator properly checks if a zone should be allocated", "[unit][zone_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    SECTION("Allocating from zoneDescIdx index, trigger not satisfied")
    {
        std::size_t idx = zoneAllocator.m_zoneDescIdx;
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == 4);
        REQUIRE(!zoneAllocator.shouldAllocateZone(idx));
    }

    SECTION("Allocating from zoneDescIdx index, trigger satisfied")
    {
        std::size_t idx = zoneAllocator.m_zoneDescIdx;
        std::size_t takeCount = zoneAllocator.m_zones.at(idx).freeChunksCount - 1;
        for (std::size_t i = 0; i < takeCount; ++i) {
            zoneAllocator.m_zones.at(idx).head->takeChunk();
            --zoneAllocator.m_zones.at(idx).freeChunksCount;
        }

        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == 1);
        REQUIRE(zoneAllocator.shouldAllocateZone(idx));
    }

    SECTION("Allocating from other index than zoneDescIdx, trigger not satisfied")
    {
        constexpr std::size_t cChunkSize = 16;
        std::size_t idx = detail::zoneIdx(cChunkSize);

        Zone zone;
        zone.clear();
        zoneAllocator.initZone(&zone, cChunkSize);
        zoneAllocator.addZone(&zone);

        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == (cPageSize / cChunkSize));
        REQUIRE(!zoneAllocator.shouldAllocateZone(idx));
    }

    SECTION("Allocating from other index than zoneDescIdx, trigger satisfied")
    {
        constexpr std::size_t cChunkSize = 16;
        std::size_t idx = detail::zoneIdx(cChunkSize);

        Zone zone;
        zone.clear();
        zoneAllocator.initZone(&zone, cChunkSize);
        zoneAllocator.addZone(&zone);

        std::size_t takeCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        for (std::size_t i = 0; i < takeCount; ++i) {
            zoneAllocator.m_zones.at(idx).head->takeChunk();
            --zoneAllocator.m_zones.at(idx).freeChunksCount;
        }
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == 0);
        REQUIRE(zoneAllocator.shouldAllocateZone(idx));
    }
}

TEST_CASE("Zone allocator properly finds free zones", "[unit][zone_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    SECTION("No free zones are available at all")
    {
        std::size_t idx = zoneAllocator.m_zoneDescIdx;
        std::size_t takeCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        for (std::size_t i = 0; i < takeCount; ++i) {
            zoneAllocator.m_zones.at(idx).head->takeChunk();
            --zoneAllocator.m_zones.at(idx).freeChunksCount;
        }

        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == 0);
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
            zoneAllocator.m_zones.at(idx).head->takeChunk();
            --zoneAllocator.m_zones.at(idx).freeChunksCount;
        }

        REQUIRE(zoneAllocator.getFreeZone(idx) == &zoneAllocator.m_initialZone);
    }
}

TEST_CASE("Zone allocator properly allocates chunks", "[unit][zone_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    SECTION("Allocate chunk from zone with index 0")
    {
        constexpr std::size_t cChunkSize = 16;
        std::size_t idx = detail::zoneIdx(cChunkSize);

        Zone zone;
        zone.clear();
        zoneAllocator.initZone(&zone, cChunkSize);
        zoneAllocator.addZone(&zone);

        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(&zone);
        REQUIRE(chunk);
        REQUIRE(zone.isValidChunk(chunk));
        REQUIRE(zone.m_freeChunksCount == zone.m_chunksCount - 1);
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount - 1);

        auto* it = zone.m_freeChunks;
        for (std::size_t i = 0; i < zone.freeChunksCount(); ++i, it = it->next())
            REQUIRE(it != chunk);
    }

    SECTION("Allocate chunk from zone with index 3")
    {
        constexpr std::size_t cChunkSize = 128;
        std::size_t idx = detail::zoneIdx(cChunkSize);

        Zone zone;
        zone.clear();
        zoneAllocator.initZone(&zone, cChunkSize);
        zoneAllocator.addZone(&zone);

        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(&zone);
        REQUIRE(chunk);
        REQUIRE(zone.isValidChunk(chunk));
        REQUIRE(zone.m_freeChunksCount == zone.m_chunksCount - 1);
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount - 1);

        auto* it = zone.m_freeChunks;
        for (std::size_t i = 0; i < zone.freeChunksCount(); ++i, it = it->next())
            REQUIRE(it != chunk);
    }
}

TEST_CASE("Zone allocator properly deallocates chunks", "[unit][zone_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    SECTION("Deallocate chunk from zone with index 0")
    {
        constexpr std::size_t cChunkSize = 16;
        std::size_t idx = detail::zoneIdx(cChunkSize);
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        auto* zone = zoneAllocator.allocateChunk<Zone>(&zoneAllocator.m_initialZone);
        zone->clear();
        zoneAllocator.initZone(zone, cChunkSize);
        zoneAllocator.addZone(zone);

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(zone);

        REQUIRE(zoneAllocator.deallocateChunk(chunk));
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Deallocate chunk from zone with index 3")
    {
        constexpr std::size_t cChunkSize = 256;
        std::size_t idx = detail::zoneIdx(cChunkSize);
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        auto* zone = zoneAllocator.allocateChunk<Zone>(&zoneAllocator.m_initialZone);
        zone->clear();
        zoneAllocator.initZone(zone, cChunkSize);
        zoneAllocator.addZone(zone);

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(zone);

        REQUIRE(zoneAllocator.deallocateChunk(chunk));
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Deallocate address from the middle of the valid chunk")
    {
        constexpr std::size_t cChunkSize = 16;
        std::size_t idx = detail::zoneIdx(cChunkSize);

        auto* zone = zoneAllocator.allocateChunk<Zone>(&zoneAllocator.m_initialZone);
        zone->clear();
        zoneAllocator.initZone(zone, cChunkSize);
        zoneAllocator.addZone(zone);

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(zone);
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        REQUIRE(!zoneAllocator.deallocateChunk(utils::movePtr(chunk, cChunkSize / 2)));
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount);
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
        constexpr std::size_t cChunkSize = 16;
        std::size_t idx = detail::zoneIdx(cChunkSize);

        auto* zone = zoneAllocator.allocateChunk<Zone>(&zoneAllocator.m_initialZone);
        zone->clear();
        zoneAllocator.initZone(zone, cChunkSize);
        zoneAllocator.addZone(zone);

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(zone);
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        zoneAllocator.allocateChunk<Chunk>(zone);

        REQUIRE(zoneAllocator.deallocateChunk(chunk));
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Deallocate whole zone with index 0")
    {
        constexpr std::size_t cChunkSize = 16;
        std::size_t idx = detail::zoneIdx(cChunkSize);
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        auto* zone = zoneAllocator.allocateChunk<Zone>(&zoneAllocator.m_initialZone);
        zone->clear();
        zoneAllocator.initZone(zone, cChunkSize);
        zoneAllocator.addZone(zone);

        std::array<Chunk*, (cPageSize / cChunkSize)> chunks{};
        for (Chunk*& chunk : chunks)
            chunk = zoneAllocator.allocateChunk<Chunk>(zone);

        for (Chunk* chunk : chunks)
            REQUIRE(zoneAllocator.deallocateChunk<Chunk>(chunk));

        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }

    SECTION("Deallocate last chunk from zone with index m_zoneDescIdx, but not from initial zone")
    {
        constexpr std::size_t cChunkSize = 64;
        std::size_t idx = detail::zoneIdx(cChunkSize);
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        REQUIRE(idx == zoneAllocator.m_zoneDescIdx);

        auto* zone = zoneAllocator.allocateChunk<Zone>(&zoneAllocator.m_initialZone);
        zone->clear();
        zoneAllocator.initZone(zone, cChunkSize);
        zoneAllocator.addZone(zone);

        std::size_t freeChunksCount = zoneAllocator.m_initialZone.freeChunksCount();
        for (std::size_t i = 0; i < freeChunksCount; ++i)
            zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);

        auto* chunk = zoneAllocator.allocateChunk<Chunk>(zone);

        REQUIRE(zoneAllocator.deallocateChunk(chunk));
        REQUIRE(zoneAllocator.m_initialZone.freeChunksCount() == 1);
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == zoneAllocator.m_initialZone.freeChunksCount());
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
    }
}

TEST_CASE("Zone allocator properly allocates zones", "[unit][zone_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    SECTION("Allocate zone with index 0, no need to preallocate a zone for zone descriptors")
    {
        constexpr std::size_t cChunkSize = 16;
        std::size_t idx = detail::zoneIdx(cChunkSize);

        auto* zone = zoneAllocator.allocateZone(cChunkSize);
        REQUIRE(zone);
        REQUIRE(zone->chunkSize() == cChunkSize);

        bool found = false;
        for (auto* it = zoneAllocator.m_zones.at(idx).head; it != nullptr; it = it->next()) {
            if (it == zone) {
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }

    SECTION("Allocate zone with index 0, need to preallocate a zone for zone descriptors")
    {
        constexpr std::size_t cChunkSize = 16;
        std::size_t idx = detail::zoneIdx(cChunkSize);

        zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);
        zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);
        zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);
        auto* zone = zoneAllocator.allocateZone(cChunkSize);
        REQUIRE(zone);
        REQUIRE(zone->chunkSize() == cChunkSize);

        bool found = false;
        for (auto* it = zoneAllocator.m_zones.at(idx).head; it != nullptr; it = it->next()) {
            if (it == zone) {
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }

    SECTION("Allocate zone with index 3, no need to preallocate a zone for zone descriptors")
    {
        constexpr std::size_t cChunkSize = 128;
        std::size_t idx = detail::zoneIdx(cChunkSize);

        auto* zone = zoneAllocator.allocateZone(cChunkSize);
        REQUIRE(zone);
        REQUIRE(zone->chunkSize() == cChunkSize);

        bool found = false;
        for (auto* it = zoneAllocator.m_zones.at(idx).head; it != nullptr; it = it->next()) {
            if (it == zone) {
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }

    SECTION("Allocate zone with index m_zoneDescIdx, no need to preallocate a zone for zone descriptors")
    {
        constexpr std::size_t cChunkSize = 64;
        std::size_t idx = detail::zoneIdx(cChunkSize);

        auto* zone = zoneAllocator.allocateZone(cChunkSize);
        REQUIRE(zone);
        REQUIRE(zone->chunkSize() == cChunkSize);

        bool found = false;
        for (auto* it = zoneAllocator.m_zones.at(idx).head; it != nullptr; it = it->next()) {
            if (it == zone) {
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }

    SECTION("Allocate zone with index 0, need to preallocate a zone for zone descriptors, no pages left")
    {
        constexpr std::size_t cChunkSize = 16;
        zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);
        zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);
        zoneAllocator.allocateChunk<Chunk>(&zoneAllocator.m_initialZone);
        REQUIRE(pageAllocator.allocate(pageAllocator.m_freePagesCount));
        REQUIRE(!zoneAllocator.allocateZone(cChunkSize));
    }

    SECTION("No pages left to initialize the new zone")
    {
        constexpr std::size_t cChunkSize = 16;
        REQUIRE(pageAllocator.allocate(pageAllocator.m_freePagesCount));
        REQUIRE(!zoneAllocator.allocateZone(cChunkSize));
    }
}

TEST_CASE("Zone allocator properly allocates user memory", "[unit][zone_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    SECTION("Allocate 0 bytes")
    {
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        REQUIRE(!zoneAllocator.allocate(0));
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == cPageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == cPageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }

    SECTION("Allocate size equal to 3 pages")
    {
        constexpr std::size_t cAllocPagesCount = 3;
        std::size_t allocSize = cAllocPagesCount * cPageSize;
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        REQUIRE(zoneAllocator.allocate(allocSize));
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount - cAllocPagesCount);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == cPageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == cPageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }

    SECTION("Allocate 6 bytes")
    {
        constexpr std::size_t cAllocSize = 6;
        std::size_t idx = detail::zoneIdx(detail::chunkSize(cAllocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        auto* ptr = zoneAllocator.allocate(cAllocSize);
        REQUIRE(ptr);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount - 1);
        std::size_t chunkSize = zoneAllocator.m_zones.at(idx).head->chunkSize();
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount + chunkSize - 1);

        bool valid = false;
        for (auto* zone = zoneAllocator.m_zones.at(idx).head; zone != nullptr; zone = zone->next()) {
            if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                valid = true;
                break;
            }
        }
        REQUIRE(valid);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == 2 * cPageSize);
        REQUIRE(stats.reservedMemorySize == zoneAllocator.m_zoneDescChunkSize);
        REQUIRE(stats.freeMemorySize == (2 * cPageSize - stats.reservedMemorySize - detail::chunkSize(cAllocSize)));
        REQUIRE(stats.allocatedMemorySize == detail::chunkSize(cAllocSize));
    }

    SECTION("Allocate 16 bytes")
    {
        constexpr std::size_t cAllocSize = 16;
        std::size_t idx = detail::zoneIdx(detail::chunkSize(cAllocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        auto* ptr = zoneAllocator.allocate(cAllocSize);
        REQUIRE(ptr);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount - 1);
        std::size_t chunkSize = zoneAllocator.m_zones.at(idx).head->chunkSize();
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount + chunkSize - 1);

        bool valid = false;
        for (auto* zone = zoneAllocator.m_zones.at(idx).head; zone != nullptr; zone = zone->next()) {
            if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                valid = true;
                break;
            }
        }
        REQUIRE(valid);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == 2 * cPageSize);
        REQUIRE(stats.reservedMemorySize == zoneAllocator.m_zoneDescChunkSize);
        REQUIRE(stats.freeMemorySize == (2 * cPageSize - stats.reservedMemorySize - detail::chunkSize(cAllocSize)));
        REQUIRE(stats.allocatedMemorySize == detail::chunkSize(cAllocSize));
    }

    SECTION("Allocate 64 bytes")
    {
        constexpr std::size_t cAllocSize = 64;
        std::size_t idx = detail::zoneIdx(detail::chunkSize(cAllocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        auto* ptr = zoneAllocator.allocate(cAllocSize);
        REQUIRE(ptr);
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount - 1);

        bool valid = false;
        for (auto* zone = zoneAllocator.m_zones.at(idx).head; zone != nullptr; zone = zone->next()) {
            if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                valid = true;
                break;
            }
        }
        REQUIRE(valid);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == cPageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == (cPageSize - stats.reservedMemorySize - detail::chunkSize(cAllocSize)));
        REQUIRE(stats.allocatedMemorySize == detail::chunkSize(cAllocSize));
    }

    SECTION("Allocate 64 bytes 6 times")
    {
        constexpr std::size_t cAllocSize = 64;
        std::size_t idx = detail::zoneIdx(detail::chunkSize(cAllocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        constexpr int cAllocCount = 6;
        std::array<void*, cAllocCount> ptrs{};
        for (void*& ptr : ptrs) {
            ptr = zoneAllocator.allocate(cAllocSize);
            REQUIRE(ptr);
        }

        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount - 1);
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == 1);

        for (void* ptr : ptrs) {
            bool valid = false;
            for (auto* zone = zoneAllocator.m_zones.at(idx).head; zone != nullptr; zone = zone->next()) {
                if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                    valid = true;
                    break;
                }
            }
            REQUIRE(valid);
        }

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == 2 * cPageSize);
        REQUIRE(stats.reservedMemorySize == zoneAllocator.m_zoneDescChunkSize);
        REQUIRE(stats.freeMemorySize == (2 * cPageSize - stats.reservedMemorySize - cAllocCount * detail::chunkSize(cAllocSize)));
        REQUIRE(stats.allocatedMemorySize == cAllocCount * detail::chunkSize(cAllocSize));
    }

    SECTION("Allocate 115 bytes 4 times")
    {
        constexpr std::size_t cAllocSize = 115;
        std::size_t idx = detail::zoneIdx(detail::chunkSize(cAllocSize));
        std::size_t freePagesCount = pageAllocator.m_freePagesCount;

        constexpr int cAllocCount = 4;
        std::array<void*, cAllocCount> ptrs{};
        for (void*& ptr : ptrs) {
            ptr = zoneAllocator.allocate(cAllocSize);
            REQUIRE(ptr);
        }

        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount - 2);
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == 0);

        for (void* ptr : ptrs) {
            bool valid = false;
            for (auto* zone = zoneAllocator.m_zones.at(idx).head; zone != nullptr; zone = zone->next()) {
                if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                    valid = true;
                    break;
                }
            }
            REQUIRE(valid);
        }

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == 3 * cPageSize);
        REQUIRE(stats.reservedMemorySize == 2 * zoneAllocator.m_zoneDescChunkSize);
        REQUIRE(stats.freeMemorySize == (3 * cPageSize - stats.reservedMemorySize - cAllocCount * detail::chunkSize(cAllocSize)));
        REQUIRE(stats.allocatedMemorySize == cAllocCount * detail::chunkSize(cAllocSize));
    }

    SECTION("Allocate 64 bytes 3 times, then 128 bytes 3 times")
    {
        // Allocate 64 bytes 3 times.
        constexpr std::size_t cAllocSize1 = 64;
        std::size_t idx1 = detail::zoneIdx(detail::chunkSize(cAllocSize1));
        std::size_t freePagesCount1 = pageAllocator.m_freePagesCount;

        constexpr int cAllocCount = 3;
        std::array<void*, cAllocCount> ptrs1{};
        for (void*& ptr : ptrs1) {
            ptr = zoneAllocator.allocate(cAllocSize1);
            REQUIRE(ptr);
        }

        for (void* ptr : ptrs1) {
            bool valid = false;
            for (auto* zone = zoneAllocator.m_zones.at(idx1).head; zone != nullptr; zone = zone->next()) {
                if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                    valid = true;
                    break;
                }
            }
            REQUIRE(valid);
        }

        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount1);
        REQUIRE(zoneAllocator.m_zones.at(zoneAllocator.m_zoneDescIdx).freeChunksCount == 1);

        // Allocate 128 bytes 3 times.
        constexpr std::size_t cAllocSize2 = 128;
        std::size_t idx2 = detail::zoneIdx(detail::chunkSize(cAllocSize2));
        std::size_t freePagesCount2 = pageAllocator.m_freePagesCount;

        std::array<void*, cAllocCount> ptrs2{};
        for (void*& ptr : ptrs2) {
            ptr = zoneAllocator.allocate(cAllocSize2);
            REQUIRE(ptr);
        }

        for (void* ptr : ptrs2) {
            bool valid = false;
            for (auto* zone = zoneAllocator.m_zones.at(idx2).head; zone != nullptr; zone = zone->next()) {
                if (zone->isValidChunk(reinterpret_cast<Chunk*>(ptr))) {
                    valid = true;
                    break;
                }
            }
            REQUIRE(valid);
        }

        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount2 - cAllocCount);
        REQUIRE(zoneAllocator.m_zones.at(zoneAllocator.m_zoneDescIdx).freeChunksCount == 2);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == 4 * cPageSize);
        REQUIRE(stats.reservedMemorySize == 3 * zoneAllocator.m_zoneDescChunkSize);
        REQUIRE(stats.freeMemorySize == (4 * cPageSize - stats.reservedMemorySize - cAllocCount * (detail::chunkSize(cAllocSize1) + detail::chunkSize(cAllocSize2))));
        REQUIRE(stats.allocatedMemorySize == cAllocCount * (detail::chunkSize(cAllocSize1) + detail::chunkSize(cAllocSize2)));
    }

    SECTION("Allocate 4 pages, no pages are available")
    {
        std::size_t allocSize = 4 * cPageSize;
        REQUIRE(pageAllocator.allocate(pageAllocator.m_freePagesCount));
        REQUIRE(!zoneAllocator.allocate(allocSize));

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == cPageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == cPageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }

    SECTION("Allocate 128 bytes, no pages are available")
    {
        constexpr std::size_t cAllocSize = 128;
        REQUIRE(pageAllocator.allocate(pageAllocator.m_freePagesCount));
        REQUIRE(!zoneAllocator.allocate(cAllocSize));

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == cPageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == cPageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }
}

TEST_CASE("Zone allocator properly releases user memory", "[unit][zone_allocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    SECTION("Release nullptr")
    {
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;
        zoneAllocator.release(nullptr);
        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
    }

    SECTION("Release invalid pointer")
    {
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;
        constexpr unsigned int cPattern = 0xdeadbeef;
        zoneAllocator.release(reinterpret_cast<void*>(cPattern));
        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
    }

    SECTION("Release memory with size equal to 3 pages")
    {
        constexpr std::size_t cAllocPagesCount = 3;
        std::size_t allocSize = cAllocPagesCount * cPageSize;
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;
        auto* ptr = zoneAllocator.allocate(allocSize);
        zoneAllocator.release(ptr);
        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
    }

    SECTION("Release 6 bytes")
    {
        constexpr std::size_t cAllocSize = 6;
        std::size_t idx = detail::zoneIdx(detail::chunkSize(cAllocSize));
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        auto* ptr = zoneAllocator.allocate(cAllocSize);
        zoneAllocator.release(ptr);
        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount);
    }

    SECTION("Release 16 bytes")
    {
        constexpr std::size_t cAllocSize = 16;
        std::size_t idx = detail::zoneIdx(detail::chunkSize(cAllocSize));
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        auto* ptr = zoneAllocator.allocate(cAllocSize);
        zoneAllocator.release(ptr);
        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount);
    }

    SECTION("Release 64 bytes")
    {
        constexpr std::size_t cAllocSize = 64;
        std::size_t idx = detail::zoneIdx(detail::chunkSize(cAllocSize));
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;
        auto* ptr = zoneAllocator.allocate(cAllocSize);
        zoneAllocator.release(ptr);
        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount);
    }

    SECTION("Release 64 bytes 6 times")
    {
        constexpr std::size_t cAllocSize = 64;
        std::size_t idx = detail::zoneIdx(detail::chunkSize(cAllocSize));
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;

        constexpr int cAllocCount = 6;
        std::array<void*, cAllocCount> ptrs{};
        for (void*& ptr : ptrs)
            ptr = zoneAllocator.allocate(cAllocSize);

        for (void* ptr : ptrs)
            zoneAllocator.release(ptr);

        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount);
    }

    SECTION("Release 115 bytes 4 times")
    {
        constexpr std::size_t cAllocSize = 115;
        std::size_t idx = detail::zoneIdx(detail::chunkSize(cAllocSize));
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;
        std::size_t freeChunksCount = zoneAllocator.m_zones.at(idx).freeChunksCount;

        constexpr int cAllocCount = 4;
        std::array<void*, cAllocCount> ptrs{};
        for (void*& ptr : ptrs)
            ptr = zoneAllocator.allocate(cAllocSize);

        for (void* ptr : ptrs)
            zoneAllocator.release(ptr);

        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones.at(idx).freeChunksCount == freeChunksCount);
    }

    SECTION("Release 64 bytes 3 times, then 128 bytes 3 times")
    {
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;

        // Allocate 64 bytes 3 times.
        constexpr std::size_t cAllocSize1 = 64;
        std::size_t idx1 = detail::zoneIdx(detail::chunkSize(cAllocSize1));
        std::size_t freeChunksCount1 = zoneAllocator.m_zones.at(idx1).freeChunksCount;

        constexpr int cAllocCount = 4;
        std::array<void*, cAllocCount> ptrs1{};
        for (void*& ptr : ptrs1)
            ptr = zoneAllocator.allocate(cAllocSize1);

        // Allocate 128 bytes 3 times.
        constexpr std::size_t cAllocSize2 = 128;
        std::size_t idx2 = detail::zoneIdx(detail::chunkSize(cAllocSize2));
        std::size_t freeChunksCount2 = zoneAllocator.m_zones.at(idx2).freeChunksCount;

        std::array<void*, cAllocCount> ptrs2{};
        for (void*& ptr : ptrs2)
            ptr = zoneAllocator.allocate(cAllocSize2);

        // Release 64 bytes 3 times.
        for (void* ptr : ptrs1)
            zoneAllocator.release(ptr);

        // Release 128 bytes 3 times.
        for (void* ptr : ptrs2)
            zoneAllocator.release(ptr);

        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
        REQUIRE(zoneAllocator.m_zones.at(idx1).freeChunksCount == freeChunksCount1);
        REQUIRE(zoneAllocator.m_zones.at(idx2).freeChunksCount == freeChunksCount2);
    }

    auto stats = zoneAllocator.getStats();
    REQUIRE(stats.usedMemorySize == cPageSize);
    REQUIRE(stats.reservedMemorySize == 0);
    REQUIRE(stats.freeMemorySize == cPageSize);
    REQUIRE(stats.allocatedMemorySize == 0);
}

} // namespace memory
