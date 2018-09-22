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
#include <random>
#include <regex>

// Make access to private members for testing.
// clang-format off
#define private     public
// clang-format on

#include <allocator/allocator.h>

using namespace memory;

TEST_CASE("Allocator returns a valid version", "[allocator]")
{
    std::regex regex("[0-9]+(\\.[0-9])+");
    REQUIRE(!std::regex_match(allocator::version(), regex));
}

TEST_CASE("Allocator is properly cleared", "[allocator]")
{
    allocator::clear();

    auto stats = allocator::getStats();
    REQUIRE(stats.totalMemorySize == 0);
    REQUIRE(stats.reservedMemorySize == 0);
    REQUIRE(stats.userMemorySize == 0);
    REQUIRE(stats.allocatedMemorySize == 0);
    REQUIRE(stats.freeMemorySize == 0);
}

TEST_CASE("Allocator is properly initialized", "[allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount1 = 535;
    std::size_t pagesCount2 = 87;
    std::size_t pagesCount3 = 4;
    auto size1 = pageSize * pagesCount1;
    auto size2 = pageSize * pagesCount2;
    auto size3 = pageSize * pagesCount3;
    auto memory1 = test::alignedAlloc(pageSize, size1);
    auto memory2 = test::alignedAlloc(pageSize, size2);
    auto memory3 = test::alignedAlloc(pageSize, size3);

    SECTION("Initialize with multiple memory regions")
    {
        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size1},
            {std::uintptr_t(memory2.get()), size2},
            {std::uintptr_t(memory3.get()), size3},
            {0,                             0}
        };
        // clang-format on

        REQUIRE(allocator::init(regions, pageSize));

        auto stats = allocator::getStats();
        REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.reservedMemorySize == 79 * pageSize);
        REQUIRE(stats.userMemorySize == (size1 + size2 + size3 - stats.reservedMemorySize));
        REQUIRE(stats.allocatedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == stats.userMemorySize);
    }

    SECTION("Initialize with single memory region")
    {
        REQUIRE(allocator::init(std::uintptr_t(memory1.get()), std::uintptr_t(memory1.get() + size1), pageSize));

        auto stats = allocator::getStats();
        REQUIRE(stats.totalMemorySize == size1);
        REQUIRE(stats.reservedMemorySize == 67 * pageSize);
        REQUIRE(stats.userMemorySize == (size1 - stats.reservedMemorySize));
        REQUIRE(stats.allocatedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == stats.userMemorySize);
    }

    SECTION("Too small number of pages in each region")
    {
        auto size = pageSize / 2;
        auto memory4 = test::alignedAlloc(pageSize, size);
        auto memory5 = test::alignedAlloc(pageSize, size);
        auto memory6 = test::alignedAlloc(pageSize, size);
        auto memory7 = test::alignedAlloc(pageSize, size);
        auto memory8 = test::alignedAlloc(pageSize, size);
        auto memory9 = test::alignedAlloc(pageSize, size);
        auto memory10 = test::alignedAlloc(pageSize, size);
        auto memory11 = test::alignedAlloc(pageSize, size);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory4.get()),  size},
            {std::uintptr_t(memory5.get()),  size},
            {std::uintptr_t(memory6.get()),  size},
            {std::uintptr_t(memory7.get()),  size},
            {std::uintptr_t(memory8.get()),  size},
            {std::uintptr_t(memory9.get()),  size},
            {std::uintptr_t(memory10.get()), size},
            {std::uintptr_t(memory11.get()), size},
            {0,                              0}
        };
        // clang-format on

        REQUIRE(!allocator::init(regions, pageSize));

        auto stats = allocator::getStats();
        REQUIRE(stats.totalMemorySize == 0);
        REQUIRE(stats.reservedMemorySize == 0);
        REQUIRE(stats.userMemorySize == 0);
        REQUIRE(stats.allocatedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == 0);
    }
}

TEST_CASE("Allocator properly allocates and releases user memory", "[allocator]")
{
    std::size_t pageSize = 256;
    std::size_t pagesCount1 = 535;
    std::size_t pagesCount2 = 87;
    std::size_t pagesCount3 = 4;
    auto size1 = pageSize * pagesCount1;
    auto size2 = pageSize * pagesCount2;
    auto size3 = pageSize * pagesCount3;
    auto memory1 = test::alignedAlloc(pageSize, size1);
    auto memory2 = test::alignedAlloc(pageSize, size2);
    auto memory3 = test::alignedAlloc(pageSize, size3);

    // clang-format off
    Region regions[] = {
        {std::uintptr_t(memory1.get()), size1},
        {std::uintptr_t(memory2.get()), size2},
        {std::uintptr_t(memory3.get()), size3},
        {0,                             0}
    };
    // clang-format on

    REQUIRE(allocator::init(regions, pageSize));

    constexpr int allocationsCount = 1000;
    constexpr int iterationsCount = 1000;
    auto maxAllocSize = 200 * pageSize;

    // Initialize random number generator.
    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());
    std::uniform_int_distribution<std::size_t> distribution(0, maxAllocSize);

    std::array<void*, allocationsCount> ptrs{};

    for (int i = 0; i < iterationsCount; ++i) {
        ptrs.fill(nullptr);

        // Allocate memory.
        for (auto*& ptr : ptrs) {
            auto allocSize = distribution(randomGenerator);
            ptr = allocator::allocate(allocSize);
        }

        // Release memory.
        for (auto* ptr : ptrs)
            allocator::release(ptr);

        auto stats = allocator::getStats();
        REQUIRE(stats.totalMemorySize == (size1 + size2 + size3));
        REQUIRE(stats.reservedMemorySize == 79 * pageSize);
        REQUIRE(stats.userMemorySize == (size1 + size2 + size3 - stats.reservedMemorySize));
        REQUIRE(stats.allocatedMemorySize == 0);
        REQUIRE(stats.freeMemorySize == stats.userMemorySize);
    }
}
