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

#include <region_info.h>

using namespace Memory;

TEST_CASE("RegionInfo structure is properly cleared", "[region_info]")
{
}

TEST_CASE("Aligned start address is properly computed", "[region_info]")
{
    SECTION("Already start-aligned region") {
    }

    SECTION("Not start-aligned region and aligned start is within region boundaries") {
    }

    SECTION("Not start-aligned region and aligned start is outside region boundaries") {
    }
}

TEST_CASE("Aligned end address is properly computed", "[region_info]")
{
    SECTION("Already end-aligned region") {
    }

    SECTION("Not end-aligned region and aligned end is within region boundaries") {
    }

    SECTION("Not end-aligned region and aligned end is outsude region boundaries") {
    }
}

TEST_CASE("RegionInfo is properly initialized", "[region_info]")
{
    SECTION("Region smaller than one page") {
    }

    SECTION("Fully aligned region, lays on 1 page") {
    }

    SECTION("Fully aligned region, lays on 5 pages") {
    }

    SECTION("Start-aligned region, lays on 1 page") {
    }

    SECTION("Start-aligned region, lays on 2 pages") {
    }

    SECTION("Start-aligned region, lays on 5 pages") {
    }

    SECTION("End-aligned region, lays on 1 page") {
    }

    SECTION("End-aligned region, lays on 2 pages") {
    }

    SECTION("End-aligned region, lays on 5 pages") {
    }

    SECTION("Fully unaligned region, lays on 1 page") {
    }

    SECTION("Fully unaligned region, lays on 2 pages") {
    }

    SECTION("End-aligned region, lays on 3 pages") {
    }

    SECTION("End-aligned region, lays on 5 pages") {
    }
}
