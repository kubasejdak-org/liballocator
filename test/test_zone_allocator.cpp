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

// Make access to private members for testing.
// clang-format off
#define private     public
// clang-format on

#include <page_allocator.h>
#include <zone_allocator.h>

#include <cstring>

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
        REQUIRE(zone.head == nullptr);
        REQUIRE(zone.freeChunksCount == 0);
    }
}
