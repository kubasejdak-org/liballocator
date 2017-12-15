////////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2017, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <iostream>

// Make access to private members for testing.
#define private     public

#include <zone_allocator/allocator.h>
#include <page_allocator.h>

using namespace Memory;

TEST_CASE("Page structure is small and natually aligned", "[page_allocator]")
{
    size_t requiredSize = 0;
    requiredSize += sizeof(Page*);          // next
    requiredSize += sizeof(Page*);          // prev
    requiredSize += sizeof(std::uintptr_t); // addr
    requiredSize += sizeof(std::uint32_t);  // flags

    REQUIRE(sizeof(Page) == requiredSize);
}

TEST_CASE("Page structure has proper layout", "[page_allocator]")
{
    REQUIRE(offsetof(Page, m_next) == 0);
    REQUIRE(offsetof(Page, m_prev) == sizeof(Page::m_next));
    REQUIRE(offsetof(Page, m_addr) == (offsetof(Page, m_prev) + sizeof(Page::m_prev)));
    REQUIRE(offsetof(Page, m_flags) == (offsetof(Page, m_addr) + sizeof(Page::m_addr)));
}

#if 0
TEST_CASE("Initialization test (single region)", "[page_allocator]")
{
    Allocator::clear();

    const std::size_t PAGE_COUNT = 4;
    const std::size_t PAGE_SIZE = 4096;
    std::array<char, PAGE_COUNT * PAGE_SIZE> memory = { 0 };

    bool result = Allocator::init(&memory[0], &memory[memory.size()]);

    SECTION("Page count is correct") {
        // If memory start is not aligned, then page count is smaller.
        auto pages = (reinterpret_cast<std::size_t>(&memory[0]) % PAGE_SIZE) ? PAGE_COUNT - 1 : PAGE_COUNT;

        REQUIRE(result);
        REQUIRE(Allocator::pageAllocator().m_allPagesCount == pages);
        REQUIRE(Allocator::pageAllocator().m_freePagesCount == pages);
    }

    SECTION("Pages are correctly linked") {
        std::size_t i = 0;
        for (auto* chains : Allocator::pageAllocator().m_freePages) {
            if (!chains)
                continue;

            for (auto* chain = chains; chain != nullptr; chain = chain->nextChain())
                i += chain->pagesCount();
        }

        REQUIRE(i == Allocator::pageAllocator().m_allPagesCount);
    }

    SECTION("All pages are within memory range") {
        for (auto* chains : Allocator::pageAllocator().m_freePages) {
            if (!chains)
                continue;

            for (auto* chain = chains; chain != nullptr; chain = chain->nextChain()) {
                for (auto* page = chain; page; page = page->nextPage()) {
                    REQUIRE(reinterpret_cast<char*>(page) >= &memory[0]);
                    REQUIRE(reinterpret_cast<char*>(page) <= &memory[memory.size()]);
                }
            }
        }
    }
}
#endif
