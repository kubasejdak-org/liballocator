/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2017-2022, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include "platformInit.hpp"

#include <allocator/allocator.hpp>

#include <fmt/printf.h>

#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Defined in linker script.
extern const char heapMin; // NOLINT
extern const char heapMax; // NOLINT

void* operator new(std::size_t size)
{
    return memory::allocator::allocate(size);
}

void operator delete(void* ptr) noexcept
{
    memory::allocator::release(ptr);
}

void operator delete(void* ptr, [[maybe_unused]] std::size_t sz) noexcept
{
    operator delete(ptr);
}

static std::size_t freeMemory()
{
    return memory::allocator::getStats().freeMemorySize;
}

//static std::size_t initialFreeMemory;

//static void showStats()
//{
//    auto stats = memory::allocator::getStats();
//    std::printf("Total memory size     : %u B\n", stats.totalMemorySize);
//    std::printf("Reserved memory size  : %u B\n", stats.reservedMemorySize);
//    std::printf("User memory size      : %u B\n", stats.userMemorySize);
//    std::printf("Allocated memory size : %u B\n", stats.allocatedMemorySize);
//    std::printf("Free memory size      : %u B\n", stats.freeMemorySize);
//    std::printf("\n");
//}
//
//static void testStart(const char* description)
//{
//    std::printf("------------------------------------------------\n");
//    std::printf("%s\n\n", description);
//    showStats();
//}
//
//static bool testEnd()
//{
//    showStats();
//
//    return (initialFreeMemory == freeMemory());
//}
//
//static bool testSmartPointers()
//{
//    testStart("Testing allocator with smart pointers");
//
//    {
//        std::printf("Allocate 113 B (std::make_unique<char[]>)...\n");
//        constexpr int cAllocSize = 113;
//        auto ptr = std::make_unique<char[]>(cAllocSize); // NOLINT
//        std::printf("ptr = %p\n", static_cast<void*>(ptr.get()));
//        showStats();
//
//        std::printf("Release memory (ptr.reset())...\n");
//    }
//
//    return testEnd();
//}
//
//static bool testVector()
//{
//    testStart("Testing allocator with std::vector<int>");
//
//    {
//        std::vector<int> vec;
//        constexpr int cItemsCount = 16;
//
//        for (int i = 0; i < cItemsCount; ++i)
//            vec.push_back(i);
//
//        for (std::size_t i = 0; i < vec.size(); ++i)
//            std::printf("vec[%u] = %d\n", i, vec[i]);
//    }
//
//    return testEnd();
//}
//
//static bool testMap()
//{
//    testStart("Testing allocator with std::map<int, std::string>");
//
//    {
//        std::map<int, std::string> map;
//        map[0] = "Great resources on modern C++:";
//        map[1] = "C++ Weekly - YouTube channel hosted by Jason Turner";
//        map[2] = "\"Effective\" books by Scott Meyers";
//        map[3] = "C++ Core Guidelines - set of guidelines, rules, and best practices about coding in C++";
//        map[4] = "CppCast - the first podcast for C++ developers by C++ developers";
//
//        for (const auto& [key, value] : map)
//            std::printf("map[%d] = %s\n", key, value.c_str());
//    }
//
//    return testEnd();
//}

// NOLINTNEXTLINE
int appMain(int argc, char* argv[])
{
    if (!platformInit())
        return EXIT_FAILURE;

    for (int i = 0; i < argc; ++i)
        fmt::print("argv[{}] = '{}'\n", i, argv[0]);

    fmt::print("PASSED\n");
    return EXIT_SUCCESS;
}
