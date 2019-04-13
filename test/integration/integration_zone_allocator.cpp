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

#include <catch2/catch.hpp>

#include <test_utils.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <random>

// Make access to private members for testing.
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define private public

#include <page_allocator.hpp>
#include <utils.hpp>
#include <zone_allocator.hpp>

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace memory;

TEST_CASE("ZoneAllocator integration tests (long-term)", "[integration][zone_allocator]")
{
    using namespace std::chrono_literals;
    constexpr auto cTestDuration = 30min;
    constexpr int cAllocationsCount = 100;

    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    // clang-format off
    std::array<Region, 2> regions = {{
        {std::uintptr_t(memory.get()), size},
        {0,                            0}
    }};
    // clang-format on

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    auto freePagesCount = pageAllocator.m_freePagesCount;
    auto maxAllocSize = 2 * cPageSize;

    // Initialize random number generator.
    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());
    std::uniform_int_distribution<std::size_t> distribution(0, maxAllocSize);

    std::array<void*, cAllocationsCount> ptrs{};

    for (auto start = test::currentTime(); !test::timeElapsed(start, cTestDuration);) {
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
        while (chunk != nullptr) {
            ++chunkCount;
            chunk = chunk->next();
        }
        REQUIRE(chunkCount == zoneAllocator.m_initialZone.freeChunksCount());
        REQUIRE(chunkCount == zoneAllocator.m_initialZone.chunksCount());
        REQUIRE(pageAllocator.m_freePagesCount == freePagesCount);

        for (const auto& zone : zoneAllocator.m_zones) {
            if (zone.head == &zoneAllocator.m_initialZone) {
                REQUIRE(zone.freeChunksCount == (cPageSize / zoneAllocator.m_zoneDescChunkSize));
                continue;
            }

            REQUIRE(zone.head == nullptr);
            REQUIRE(zone.freeChunksCount == 0);
        }

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == cPageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == cPageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }
}
