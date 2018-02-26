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

#include "catch.hpp"
#include "utils.h"

// Make access to private members for testing.
// clang-format off
#define private     public
// clang-format on

#include <page_allocator.h>

#include <cstdlib>
#include <cstring>

using namespace Memory;

TEST_CASE("Page allocator is properly cleared", "[page_allocator]")
{
    PageAllocator pageAllocator;
    std::memset(&pageAllocator, 0x5a, sizeof(PageAllocator));

    pageAllocator.clear();
    for (const auto& region : pageAllocator.m_regionsInfo) {
        REQUIRE(region.start == 0);
        REQUIRE(region.end == 0);
        REQUIRE(region.alignedStart == 0);
        REQUIRE(region.alignedEnd == 0);
        REQUIRE(region.pageCount == 0);
        REQUIRE(region.size == 0);
        REQUIRE(region.alignedSize == 0);
        REQUIRE(region.firstPage == nullptr);
        REQUIRE(region.lastPage == nullptr);
    }
    REQUIRE(pageAllocator.m_validRegionsCount == 0);
    REQUIRE(pageAllocator.m_descRegionIdx == 0);
    REQUIRE(pageAllocator.m_descPagesCount == 0);
    REQUIRE(pageAllocator.m_pagesHead == nullptr);
    REQUIRE(pageAllocator.m_pagesTail == nullptr);
    for (const auto* page : pageAllocator.m_freeGroupLists)
        REQUIRE(page == nullptr);
    REQUIRE(pageAllocator.m_pagesCount == 0);
    REQUIRE(pageAllocator.m_freePagesCount == 0);
}

TEST_CASE("Pages are correctly counted", "[page_allocator]")
{
    std::size_t pageSize = 256;
    PageAllocator pageAllocator;

    SECTION("Regions: 1(1)")
    {
        std::size_t pagesCount = 1;
        auto size = pageSize * pagesCount;
        auto memory = test_alignedAlloc(pageSize, size);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory.get()), size},
            {0,                            0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_pagesCount == pagesCount);
    }

    SECTION("Regions: 1(535), 2(87), 3(4)")
    {
        std::size_t pageSize = 256;
        std::size_t pagesCount1 = 535;
        std::size_t pagesCount2 = 87;
        std::size_t pagesCount3 = 4;
        auto size1 = pageSize * pagesCount1;
        auto size2 = pageSize * pagesCount2;
        auto size3 = pageSize * pagesCount3;
        auto memory1 = test_alignedAlloc(pageSize, size1);
        auto memory2 = test_alignedAlloc(pageSize, size2);
        auto memory3 = test_alignedAlloc(pageSize, size3);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size1},
            {std::uintptr_t(memory2.get()), size2},
            {std::uintptr_t(memory3.get()), size3},
            {0,                             0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_pagesCount == (pagesCount1 + pagesCount2 + pagesCount3));
    }

    SECTION("All regions have 5 pages")
    {
        std::size_t pageSize = 256;
        std::size_t pagesCount = 5;
        auto size = pageSize * pagesCount;
        auto memory1 = test_alignedAlloc(pageSize, size);
        auto memory2 = test_alignedAlloc(pageSize, size);
        auto memory3 = test_alignedAlloc(pageSize, size);
        auto memory4 = test_alignedAlloc(pageSize, size);
        auto memory5 = test_alignedAlloc(pageSize, size);
        auto memory6 = test_alignedAlloc(pageSize, size);
        auto memory7 = test_alignedAlloc(pageSize, size);
        auto memory8 = test_alignedAlloc(pageSize, size);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size},
            {std::uintptr_t(memory2.get()), size},
            {std::uintptr_t(memory3.get()), size},
            {std::uintptr_t(memory4.get()), size},
            {std::uintptr_t(memory5.get()), size},
            {std::uintptr_t(memory6.get()), size},
            {std::uintptr_t(memory7.get()), size},
            {std::uintptr_t(memory8.get()), size},
            {0,                              0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_pagesCount == (pagesCount * 8));
    }
}

#include <iostream>
TEST_CASE("Region where page descriptors are stored is properly selected", "[page_allocator]")
{
    std::size_t pageSize = 256;
    PageAllocator pageAllocator;

    SECTION("Regions: 1(1)")
    {
        std::size_t pagesCount = 1;
        auto size = pageSize * pagesCount;
        auto memory = test_alignedAlloc(pageSize, size);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory.get()), size},
            {0,                            0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_descRegionIdx == 0);

    }

    SECTION("Regions: 1(535), 2(87), 3(4)")
    {
        std::size_t pageSize = 256;
        std::size_t pagesCount1 = 535;
        std::size_t pagesCount2 = 87;
        std::size_t pagesCount3 = 4;
        auto size1 = pageSize * pagesCount1;
        auto size2 = pageSize * pagesCount2;
        auto size3 = pageSize * pagesCount3;
        auto memory1 = test_alignedAlloc(pageSize, size1);
        auto memory2 = test_alignedAlloc(pageSize, size2);
        auto memory3 = test_alignedAlloc(pageSize, size3);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size1},
            {std::uintptr_t(memory2.get()), size2},
            {std::uintptr_t(memory3.get()), size3},
            {0,                             0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_descRegionIdx == 1);
    }

    SECTION("All regions have 5 pages")
    {
        std::size_t pageSize = 256;
        std::size_t pagesCount = 5;
        auto size = pageSize * pagesCount;
        auto memory1 = test_alignedAlloc(pageSize, size);
        auto memory2 = test_alignedAlloc(pageSize, size);
        auto memory3 = test_alignedAlloc(pageSize, size);
        auto memory4 = test_alignedAlloc(pageSize, size);
        auto memory5 = test_alignedAlloc(pageSize, size);
        auto memory6 = test_alignedAlloc(pageSize, size);
        auto memory7 = test_alignedAlloc(pageSize, size);
        auto memory8 = test_alignedAlloc(pageSize, size);

        // clang-format off
        Region regions[] = {
            {std::uintptr_t(memory1.get()), size},
            {std::uintptr_t(memory2.get()), size},
            {std::uintptr_t(memory3.get()), size},
            {std::uintptr_t(memory4.get()), size},
            {std::uintptr_t(memory5.get()), size},
            {std::uintptr_t(memory6.get()), size},
            {std::uintptr_t(memory7.get()), size},
            {std::uintptr_t(memory8.get()), size},
            {0,                              0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_descRegionIdx == 0);
    }

    SECTION("Selected region is completly filled")
    {
        std::size_t pageSize = 256;
        std::size_t pagesCount1 = 1;
        std::size_t pagesCount2 = 7;
        auto size1 = pageSize * pagesCount1;
        auto size2 = pageSize * pagesCount2;
        auto memory1 = test_alignedAlloc(pageSize, size1);
        auto memory2 = test_alignedAlloc(pageSize, size2);

        // clang-format off
        Region regions[] = {
                {std::uintptr_t(memory1.get()), size1},
                {std::uintptr_t(memory2.get()), size2},
                {0,                             0}
        };
        // clang-format on

        REQUIRE(pageAllocator.init(regions, pageSize));
        REQUIRE(pageAllocator.m_descRegionIdx == 0);
    }
}

TEST_CASE("Pages with page descriptors are properly reserved", "[page_allocator]")
{
    SECTION("All descriptors lay on 1 page in first region")
    {
    }

    SECTION("All descriptors lay on 3 pages in first region")
    {
    }

    SECTION("All descriptors lay on 1 page in third region")
    {
    }

    SECTION("All descriptors lay on 3 pages in third region")
    {
    }
}

TEST_CASE("Group index is properly computed", "[page_allocator]")
{
}

TEST_CASE("Group is properly initialized", "[page_allocator]")
{
    SECTION("Group has 1 page")
    {
    }

    SECTION("Group has 5 pages")
    {
    }
}

TEST_CASE("Group is properly cleared", "[page_allocator]")
{
    SECTION("Group has 1 page")
    {
    }

    SECTION("Group has 5 pages")
    {
    }
}

TEST_CASE("Group is properly added to list", "[page_allocator]")
{
    SECTION("Group is stored at index 0")
    {
    }

    SECTION("Group is stored at index 4")
    {
    }
}

TEST_CASE("Group is properly removed from list", "[page_allocator]")
{
    SECTION("Group is stored at index 0")
    {
    }

    SECTION("Group is stored at index 4")
    {
    }
}

// TODO:
// - tests for splitting group
// - tests for joining groups
// - tests for initialization
// - tests for allocation
// - tests for releasing
// - integration tests (long-term)
