/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2017-2023, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <Page.hpp>
#include <PageAllocator.hpp>
#include <TestUtils.hpp>
#include <ZoneAllocator.hpp>
#include <allocator/Region.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <utility>

namespace memory {

TEST_CASE("ZoneAllocator is properly cleared", "[unit][ZoneAllocator]")
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

TEST_CASE("ZoneAllocator is properly initialized", "[unit][ZoneAllocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {
        {{std::uintptr_t(memory.get()), size}, {0, 0}}
    };

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

TEST_CASE("ZoneAllocator stats are properly initialized", "[unit][ZoneAllocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {
        {{std::uintptr_t(memory.get()), size}, {0, 0}}
    };

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    auto stats = zoneAllocator.getStats();
    REQUIRE(stats.usedMemorySize == cPageSize);
    REQUIRE(stats.reservedMemorySize == 0);
    REQUIRE(stats.freeMemorySize == cPageSize);
    REQUIRE(stats.allocatedMemorySize == 0);
}

TEST_CASE("Chunk size is properly calculated", "[unit][ZoneAllocator]")
{
    std::size_t size = 0;
    std::size_t roundedSize = 0;

    SECTION("Size is smaller than the minimal alloc size")
    {
        size = ZoneAllocator::minimalAllocSize() / 2;
        roundedSize = detail::chunkSize(size);
        REQUIRE(roundedSize == ZoneAllocator::minimalAllocSize());
    }

    SECTION("Size is equal to the minimal alloc size")
    {
        size = ZoneAllocator::minimalAllocSize();
        roundedSize = detail::chunkSize(size);
        REQUIRE(roundedSize == ZoneAllocator::minimalAllocSize());
    }

    SECTION("Size is greater than the minimal alloc size")
    {
        size = ZoneAllocator::minimalAllocSize() * 2;
        roundedSize = detail::chunkSize(size);
        REQUIRE(roundedSize > ZoneAllocator::minimalAllocSize());
    }

    REQUIRE(roundedSize >= size);
    REQUIRE(std::pow(2.0, std::log2(double(roundedSize))) == double(roundedSize)); // NOLINT
}

TEST_CASE("Zone index is properly calculated", "[unit][ZoneAllocator]")
{
    std::map<std::size_t, std::pair<std::size_t, std::size_t>> idxRange = {
        {0, {16, 31}    }, // NOLINT
        {1, {32, 63}    }, // NOLINT
        {2, {64, 127}   }, // NOLINT
        {3, {128, 255}  }, // NOLINT
        {4, {256, 511}  }, // NOLINT
        {5, {512, 1023} }, // NOLINT
        {6, {1024, 2047}}, // NOLINT
        {7, {2048, 4095}}  // NOLINT
    };

    for (std::size_t i = idxRange[0].first; i < idxRange[idxRange.size() - 1].second; ++i) {
        auto idx = detail::zoneIdx(i);
        REQUIRE(i >= idxRange[idx].first);
        REQUIRE(i <= idxRange[idx].second);
    }
}

TEST_CASE("Zone allocator properly allocates user memory", "[unit][ZoneAllocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {
        {{std::uintptr_t(memory.get()), size}, {0, 0}}
    };

    REQUIRE(pageAllocator.init(regions.data(), cPageSize));

    ZoneAllocator zoneAllocator;
    REQUIRE(zoneAllocator.init(&pageAllocator, cPageSize));

    SECTION("Allocate 0 bytes")
    {
        REQUIRE(!zoneAllocator.allocate(0));

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
        REQUIRE(zoneAllocator.allocate(allocSize));

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == cPageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == cPageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }

    SECTION("Allocate 6 bytes")
    {
        constexpr std::size_t cAllocSize = 6;
        auto* ptr = zoneAllocator.allocate(cAllocSize);
        REQUIRE(ptr);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.allocatedMemorySize == detail::chunkSize(cAllocSize));
    }

    SECTION("Allocate 16 bytes")
    {
        constexpr std::size_t cAllocSize = 16;
        auto* ptr = zoneAllocator.allocate(cAllocSize);
        REQUIRE(ptr);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.allocatedMemorySize == detail::chunkSize(cAllocSize));
    }

    SECTION("Allocate 64 bytes")
    {
        constexpr std::size_t cAllocSize = 64;
        auto* ptr = zoneAllocator.allocate(cAllocSize);
        REQUIRE(ptr);

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.allocatedMemorySize == detail::chunkSize(cAllocSize));
    }

    SECTION("Allocate 64 bytes 6 times")
    {
        constexpr std::size_t cAllocSize = 64;
        constexpr int cAllocCount = 6;
        std::array<void*, cAllocCount> ptrs{};
        for (void*& ptr : ptrs) {
            ptr = zoneAllocator.allocate(cAllocSize);
            REQUIRE(ptr);
        }

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.allocatedMemorySize == cAllocCount * detail::chunkSize(cAllocSize));
    }

    SECTION("Allocate 115 bytes 4 times")
    {
        constexpr std::size_t cAllocSize = 115;
        constexpr int cAllocCount = 4;
        std::array<void*, cAllocCount> ptrs{};
        for (void*& ptr : ptrs) {
            ptr = zoneAllocator.allocate(cAllocSize);
            REQUIRE(ptr);
        }

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.allocatedMemorySize == cAllocCount * detail::chunkSize(cAllocSize));
    }

    SECTION("Allocate 64 bytes 3 times, then 128 bytes 3 times")
    {
        // Allocate 64 bytes 3 times.
        constexpr std::size_t cAllocSize1 = 64;
        constexpr int cAllocCount = 3;
        std::array<void*, cAllocCount> ptrs1{};
        for (void*& ptr : ptrs1) {
            ptr = zoneAllocator.allocate(cAllocSize1);
            REQUIRE(ptr);
        }

        // Allocate 128 bytes 3 times.
        constexpr std::size_t cAllocSize2 = 128;
        std::array<void*, cAllocCount> ptrs2{};
        for (void*& ptr : ptrs2) {
            ptr = zoneAllocator.allocate(cAllocSize2);
            REQUIRE(ptr);
        }

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.allocatedMemorySize
                == cAllocCount * (detail::chunkSize(cAllocSize1) + detail::chunkSize(cAllocSize2)));
    }

    SECTION("Allocate 4 pages, no pages are available")
    {
        std::size_t allocSize = 4 * cPageSize;
        REQUIRE(pageAllocator.allocate(pageAllocator.getStats().freePagesCount));
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
        REQUIRE(pageAllocator.allocate(pageAllocator.getStats().freePagesCount));
        REQUIRE(!zoneAllocator.allocate(cAllocSize));

        auto stats = zoneAllocator.getStats();
        REQUIRE(stats.usedMemorySize == cPageSize);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == cPageSize);
        REQUIRE(stats.allocatedMemorySize == 0);
    }
}

TEST_CASE("Zone allocator properly releases user memory", "[unit][ZoneAllocator]")
{
    constexpr std::size_t cPageSize = 256;
    constexpr std::size_t cPagesCount = 256;
    PageAllocator pageAllocator;

    auto size = cPageSize * cPagesCount;
    auto memory = test::alignedAlloc(cPageSize, size);

    constexpr int cRegionsCount = 2;
    std::array<Region, cRegionsCount> regions = {
        {{std::uintptr_t(memory.get()), size}, {0, 0}}
    };

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
        constexpr std::size_t cAllocSize = cAllocPagesCount * cPageSize;
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;
        auto* ptr = zoneAllocator.allocate(cAllocSize);

        constexpr int cMemsetPattern = 0x5a;
        std::memset(ptr, cMemsetPattern, cAllocSize);

        zoneAllocator.release(ptr);
        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
    }

    SECTION("Release 6 bytes")
    {
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;

        constexpr std::size_t cAllocSize = 6;
        auto* ptr = zoneAllocator.allocate(cAllocSize);

        constexpr int cMemsetPattern = 0x5a;
        std::memset(ptr, cMemsetPattern, cAllocSize);

        zoneAllocator.release(ptr);
        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
    }

    SECTION("Release 16 bytes")
    {
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;

        constexpr std::size_t cAllocSize = 16;
        auto* ptr = zoneAllocator.allocate(cAllocSize);

        constexpr int cMemsetPattern = 0x5a;
        std::memset(ptr, cMemsetPattern, cAllocSize);

        zoneAllocator.release(ptr);
        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
    }

    SECTION("Release 64 bytes")
    {
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;

        constexpr std::size_t cAllocSize = 64;
        auto* ptr = zoneAllocator.allocate(cAllocSize);

        constexpr int cMemsetPattern = 0x5a;
        std::memset(ptr, cMemsetPattern, cAllocSize);

        zoneAllocator.release(ptr);
        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
    }

    SECTION("Release 64 bytes 6 times")
    {
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;

        constexpr std::size_t cAllocSize = 64;
        constexpr int cAllocCount = 6;
        std::array<void*, cAllocCount> ptrs{};
        for (void*& ptr : ptrs) {
            ptr = zoneAllocator.allocate(cAllocSize);

            constexpr int cMemsetPattern = 0x5a;
            std::memset(ptr, cMemsetPattern, cAllocSize);
        }

        for (void* ptr : ptrs)
            zoneAllocator.release(ptr);

        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
    }

    SECTION("Release 115 bytes 4 times")
    {
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;

        constexpr std::size_t cAllocSize = 115;
        constexpr int cAllocCount = 4;
        std::array<void*, cAllocCount> ptrs{};
        for (void*& ptr : ptrs) {
            ptr = zoneAllocator.allocate(cAllocSize);

            constexpr int cMemsetPattern = 0x5a;
            std::memset(ptr, cMemsetPattern, cAllocSize);
        }

        for (void* ptr : ptrs)
            zoneAllocator.release(ptr);

        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
    }

    SECTION("Release 64 bytes 3 times, then 128 bytes 3 times")
    {
        std::size_t freePagesCount = pageAllocator.getStats().freePagesCount;

        // Allocate 64 bytes 3 times.
        constexpr std::size_t cAllocSize1 = 64;
        constexpr int cAllocCount = 4;
        std::array<void*, cAllocCount> ptrs1{};
        for (void*& ptr : ptrs1) {
            ptr = zoneAllocator.allocate(cAllocSize1);

            constexpr int cMemsetPattern = 0x5a;
            std::memset(ptr, cMemsetPattern, cAllocSize1);
        }

        // Allocate 128 bytes 3 times.
        constexpr std::size_t cAllocSize2 = 128;
        std::array<void*, cAllocCount> ptrs2{};
        for (void*& ptr : ptrs2) {
            ptr = zoneAllocator.allocate(cAllocSize2);

            constexpr int cMemsetPattern = 0x5a;
            std::memset(ptr, cMemsetPattern, cAllocSize2);
        }

        // Release 64 bytes 3 times.
        for (void* ptr : ptrs1)
            zoneAllocator.release(ptr);

        // Release 128 bytes 3 times.
        for (void* ptr : ptrs2)
            zoneAllocator.release(ptr);

        REQUIRE(pageAllocator.getStats().freePagesCount == freePagesCount);
    }

    auto stats = zoneAllocator.getStats();
    REQUIRE(stats.usedMemorySize == cPageSize);
    REQUIRE(stats.reservedMemorySize == 0);
    REQUIRE(stats.freeMemorySize == cPageSize);
    REQUIRE(stats.allocatedMemorySize == 0);
}

} // namespace memory
