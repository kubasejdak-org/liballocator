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

#include "page_allocator.h"
#include "zone_allocator.h"

#include <zone_allocator/allocator.h>

namespace Memory {

bool Allocator::init(Region *regions, std::size_t pageSize)
{
    if (!pageAllocator().init(regions, pageSize))
        return false;

    return zoneAllocator().init(&pageAllocator());
}

bool Allocator::init(char *start, char *end, std::size_t pageSize)
{
    Region regions[2] = {
        { .address = start  , .size = static_cast<std::size_t>(end - start) },
        { .address = nullptr, .size = 0                                     }
    };

    return init(regions, pageSize);
}

void Allocator::clear()
{
    pageAllocator().clear();
    zoneAllocator().clear();
}

void *Allocator::allocate(std::size_t size)
{
    return zoneAllocator().allocate(size);
}

void Allocator::release(void *ptr)
{
    zoneAllocator().release(ptr);
}

PageAllocator &Allocator::pageAllocator()
{
    static PageAllocator pageAllocator;
    return pageAllocator;
}

ZoneAllocator &Allocator::zoneAllocator()
{
    static ZoneAllocator zoneAllocator;
    return zoneAllocator;
}

} // namespace Memory