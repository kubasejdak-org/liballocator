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

#ifndef REGION_INFO_H
#define REGION_INFO_H

#include "page.h"

#include <zone_allocator/region.h>

#include <cstddef>
#include <cstdint>
#include <optional>

namespace Memory {

struct RegionInfo {
    std::uintptr_t start;
    std::uintptr_t end;
    std::uintptr_t alignedStart;
    std::uintptr_t alignedEnd;
    std::size_t pageCount;
    std::size_t size;
    std::size_t alignedSize;
    Page* firstPage;
    Page* lastPage;
};

void clearRegionInfo(RegionInfo& regionInfo);
bool initRegionInfo(RegionInfo& regionInfo, Region& region, std::size_t pageSize);

namespace detail {

std::optional<std::uintptr_t> alignedStart(Region& region, std::size_t pageSize);
std::optional<std::uintptr_t> alignedEnd(Region& region, std::size_t pageSize);

} // namespace detail
} // namespace Memory

#endif
