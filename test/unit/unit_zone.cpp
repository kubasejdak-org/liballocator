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

#include <test_utils.h>

#include <array>
#include <cstddef>

// Make access to private members for testing.
// clang-format off
#define private     public
// clang-format on

#include <page.h>
#include <zone.h>

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace memory;

TEST_CASE("Zone structure is naturally aligned", "[unit][zone]")
{
    REQUIRE(Zone::isNaturallyAligned());
}

TEST_CASE("Zone is properly initialized", "[unit][zone]")
{
    std::size_t pageSize = 256;
    auto memory = test::alignedAlloc(pageSize, pageSize);

    std::byte buffer[sizeof(Page)];
    auto* page = reinterpret_cast<Page*>(buffer);
    page->setAddress(std::uintptr_t(memory.get()));

    Zone zone;
    std::size_t chunkSize = 64;

    zone.init(page, pageSize, chunkSize);
    REQUIRE(zone.m_next == nullptr);
    REQUIRE(zone.m_prev == nullptr);
    REQUIRE(zone.m_page == page);
    REQUIRE(zone.m_chunkSize == chunkSize);
    REQUIRE(zone.m_chunksCount == (pageSize / chunkSize));
    REQUIRE(zone.m_freeChunksCount == (pageSize / chunkSize));
    std::uintptr_t lastChunkAddr = page->address() + pageSize - chunkSize;
    REQUIRE(zone.m_freeChunks == reinterpret_cast<Chunk*>(lastChunkAddr));

    REQUIRE(zone.page() == page);
    REQUIRE(zone.chunkSize() == chunkSize);
    REQUIRE(zone.chunksCount() == (pageSize / chunkSize));
    REQUIRE(zone.freeChunksCount() == (pageSize / chunkSize));

    auto* chunk = reinterpret_cast<Chunk*>(zone.page()->address());
    for (std::size_t i = 0; i < zone.chunksCount(); ++i) {
        REQUIRE(std::uintptr_t(chunk) == zone.page()->address() + i * chunkSize);
        chunk = chunk->m_prev;
    }
}

TEST_CASE("Zone is properly cleared", "[unit][zone]")
{
    Zone zone;
    zone.clear();

    REQUIRE(zone.m_next == nullptr);
    REQUIRE(zone.m_prev == nullptr);
    REQUIRE(zone.m_page == nullptr);
    REQUIRE(zone.m_chunkSize == 0);
    REQUIRE(zone.m_chunksCount == 0);
    REQUIRE(zone.m_freeChunksCount == 0);
    REQUIRE(zone.m_freeChunks == nullptr);

    REQUIRE(zone.page() == nullptr);
    REQUIRE(zone.chunkSize() == 0);
    REQUIRE(zone.chunksCount() == 0);
    REQUIRE(zone.freeChunksCount() == 0);
}

TEST_CASE("Zone properly allocates chunks", "[unit][zone]")
{
    std::size_t pageSize = 256;
    auto memory = test::alignedAlloc(pageSize, pageSize);

    std::byte buffer[sizeof(Page)];
    auto* page = reinterpret_cast<Page*>(buffer);
    page->setAddress(std::uintptr_t(memory.get()));

    Zone zone;
    std::size_t chunkSize = 64;
    zone.init(page, pageSize, chunkSize);

    std::size_t chunksCount = zone.chunksCount();
    std::size_t freeChunksCount = zone.chunksCount();
    for (std::size_t i = 0; i < zone.chunksCount(); ++i) {
        --freeChunksCount;
        auto* chunk = zone.takeChunk();
        REQUIRE(chunk);
        REQUIRE(std::uintptr_t(chunk) == zone.page()->address() + pageSize - chunkSize * (1 + i));
        REQUIRE(zone.chunksCount() == chunksCount);
        REQUIRE(zone.freeChunksCount() == freeChunksCount);
    }

    REQUIRE(zone.freeChunksCount() == 0);
}

TEST_CASE("Zone properly deallocates chunks", "[unit][zone]")
{
    constexpr std::size_t pageSize = 256;
    auto memory = test::alignedAlloc(pageSize, pageSize);

    std::byte buffer[sizeof(Page)];
    auto* page = reinterpret_cast<Page*>(buffer);
    page->setAddress(std::uintptr_t(memory.get()));

    Zone zone;
    constexpr std::size_t chunkSize = 64;
    zone.init(page, pageSize, chunkSize);

    std::array<Chunk*, (pageSize / chunkSize)> chunks{};

    for (std::size_t i = 0; i < zone.chunksCount(); ++i)
        chunks.at(i) = zone.takeChunk();

    SECTION("Release in order")
    {
        for (std::size_t i = 0; i < zone.chunksCount(); ++i)
            zone.giveChunk(chunks.at(i));
    }

    SECTION("Release in reverse order")
    {
        for (std::size_t i = 0; i < zone.chunksCount(); ++i)
            zone.giveChunk(chunks.at(zone.chunksCount() - 1 - i));
    }

    SECTION("Release in custom order")
    {
        zone.giveChunk(chunks[2]);
        zone.giveChunk(chunks[0]);
        zone.giveChunk(chunks[3]);
        zone.giveChunk(chunks[1]);
    }

    REQUIRE(zone.chunksCount() == (pageSize / chunkSize));
    REQUIRE(zone.freeChunksCount() == (pageSize / chunkSize));
}

TEST_CASE("Zone properly checks if given zone is valid", "[unit][zone]")
{
    constexpr std::size_t pageSize = 256;
    auto memory = test::alignedAlloc(pageSize, pageSize);

    std::byte buffer[sizeof(Page)];
    auto* page = reinterpret_cast<Page*>(buffer);
    page->setAddress(std::uintptr_t(memory.get()));

    Zone zone;
    constexpr std::size_t chunkSize = 64;
    zone.init(page, pageSize, chunkSize);

    std::array<Chunk*, (pageSize / chunkSize)> chunks{};

    for (std::size_t i = 0; i < zone.chunksCount(); ++i)
        chunks.at(i) = zone.takeChunk();

    SECTION("Check all valid chunks")
    {
        for (std::size_t i = 0; i < zone.chunksCount(); ++i)
            REQUIRE(zone.isValidChunk(chunks.at(i)));
    }

    SECTION("Check address from the middle of the valid chunk")
    {
        std::uintptr_t addr = std::uintptr_t(chunks[0]) + chunkSize / 2;
        REQUIRE(!zone.isValidChunk(reinterpret_cast<Chunk*>(addr)));
    }

    SECTION("Check address of the last byte in the valid chunk")
    {
        std::uintptr_t addr = std::uintptr_t(chunks[1]) - 1;
        REQUIRE(!zone.isValidChunk(reinterpret_cast<Chunk*>(addr)));
    }

    SECTION("Check address lower than the zone start")
    {
        std::uintptr_t addr = zone.page()->address() - 1;
        REQUIRE(!zone.isValidChunk(reinterpret_cast<Chunk*>(addr)));
    }

    SECTION("Check address higher than the zone end")
    {
        std::uintptr_t addr = zone.page()->address() + pageSize + 1;
        REQUIRE(!zone.isValidChunk(reinterpret_cast<Chunk*>(addr)));
    }

    SECTION("Check nullptr")
    {
        REQUIRE(!zone.isValidChunk(nullptr));
    }

    SECTION("Check invalid address")
    {
        REQUIRE(!zone.isValidChunk(reinterpret_cast<Chunk*>(0xdeadbeef)));
    }
}
