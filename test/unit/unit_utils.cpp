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

#include <array>
#include <cmath>
#include <cstddef>

// Make access to private members for testing.
// clang-format off
#define private     public
// clang-format on

#include <utils.h>
#include <zone.h>

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace memory;

TEST_CASE("Values are correctly rounded to the closest power of 2", "[unit][utils]")
{
    double idx = 0.0;

    for (std::size_t i = 1; i < 1000000; ++i) {
        double requiredValue = std::pow(2.0, idx);
        auto value = utils::roundPower2(i);
        REQUIRE(value == requiredValue);

        if (std::log2(double(i)) == idx)
            ++idx;
    }
}

TEST_CASE("Pointers are correctly moved", "[unit][utils]")
{
    SECTION("Pointer has the type 'char'")
    {
        std::array<std::byte, 64> memory{};

        for (std::size_t i = 0; i < memory.size(); ++i) {
            auto* ptr = reinterpret_cast<char*>(&memory[0]);
            REQUIRE(utils::movePtr(ptr, i) == reinterpret_cast<char*>(&memory.at(i)));
        }
    }

    SECTION("Pointer has the type 'long double'")
    {
        std::array<std::byte, 64> memory{};

        for (std::size_t i = 0; i < memory.size(); ++i) {
            auto* ptr = reinterpret_cast<long double*>(&memory[0]);
            REQUIRE(utils::movePtr(ptr, i) == reinterpret_cast<long double*>(&memory.at(i)));
        }
    }

    SECTION("Pointer has the type 'Chunk'")
    {
        std::array<std::byte, 64> memory{};

        for (std::size_t i = 0; i < memory.size(); ++i) {
            auto* ptr = reinterpret_cast<Chunk*>(&memory[0]);
            REQUIRE(utils::movePtr(ptr, i) == reinterpret_cast<Chunk*>(&memory.at(i)));
        }
    }
}