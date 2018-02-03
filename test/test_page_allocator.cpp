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

#include <page_allocator.h>

using namespace Memory;

TEST_CASE("Page allocator is properly cleared", "[page_allocator]")
{
}

TEST_CASE("Pages are correctly counted", "[page_allocator]")
{
    SECTION("All regions are cleared")
    {
    }

    SECTION("Regions: 1(1)")
    {
    }

    SECTION("Regions: 1(535), 2(87), 3(4)")
    {
    }

    SECTION("All regions have 5 pages")
    {
    }
}

TEST_CASE("Region where page descriptors are stored is properly selected", "[page_allocator]")
{
    SECTION("Regions: 1(1)")
    {
    }

    SECTION("Regions: 1(535), 2(87), 3(4)")
    {
    }

    SECTION("All regions have 5 pages")
    {
    }

    SECTION("Selected region is completly filled")
    {
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
