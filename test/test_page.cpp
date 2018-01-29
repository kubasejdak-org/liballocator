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

// Make access to private members for testing.
// clang-format off
#define private     public
// clang-format on

#include <page.h>

using namespace Memory;

TEST_CASE("Page structure is naturally aligned", "[page]")
{
    std::size_t requiredSize = 0;
    requiredSize += sizeof(Page*);          // m_nextGroup
    requiredSize += sizeof(Page*);          // m_prevGroup
    requiredSize += sizeof(std::uintptr_t); // m_addr
    requiredSize += sizeof(Page::Flags);    // m_flags

    REQUIRE(sizeof(Page) == requiredSize);
}

TEST_CASE("Page is properly initialized", "[page]")
{
}

TEST_CASE("Accessing siblings works as expected", "[page]")
{
    SECTION("Previous sibling") {
    }

    SECTION("Next sibling") {
    }
}

TEST_CASE("Adding to empty list", "[page]")
{
}

TEST_CASE("Adding to non-empty list", "[page]")
{
}

TEST_CASE("Removing from list with 5 pages", "[page]")
{
    SECTION("Removing first page") {
    }

    SECTION("Removing middle page") {
    }

    SECTION("Removing last page") {
    }

    SECTION("Removing all pages") {
    }
}

TEST_CASE("Removing from list with 1 page", "[page]")
{
}
