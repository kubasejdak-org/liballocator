/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2017-2020, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <cstdio>
#include <cstdlib>

// Defined in linker script.
extern char heapMin;
extern char heapMax;

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

static void showStats()
{
    auto stats = memory::allocator::getStats();
    std::printf("Total memory size     : %u B\n", stats.totalMemorySize);
    std::printf("Reserved memory size  : %u B\n", stats.reservedMemorySize);
    std::printf("User memory size      : %u B\n", stats.userMemorySize);
    std::printf("Allocated memory size : %u B\n", stats.allocatedMemorySize);
    std::printf("Free memory size      : %u B\n", stats.freeMemorySize);
    std::printf("\n");
}

// NOLINTNEXTLINE
int appMain([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    if (!platformInit())
        return EXIT_FAILURE;

    if (!memory::allocator::init(std::uintptr_t(&heapMin), std::uintptr_t(&heapMax), 4096)) {
        std::printf("error: Failed to initialize liballocator.\n");
        while (true) {
            // Empty.
        }
    }

    std::printf("Initialized liballocator v%s.\n", memory::allocator::version());
    showStats();

    std::printf("Allocate 113 B (new char[113])... ");
    char* ptr = new char[113];
    std::printf("ptr = %p\n", ptr);
    showStats();

    std::printf("Release ptr (delete [] ptr)...\n");
    delete[] ptr;
    showStats();

    while (true) {
        // Empty.
    }

    return 0;
}
