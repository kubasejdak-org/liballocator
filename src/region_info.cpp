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

#include "region_info.h"

namespace Memory {

std::uintptr_t alignedStart(Region& region, std::size_t pageSize)
{
    auto start = region.address & ~(pageSize - 1);
    if (start < region.address)
        start += pageSize;

    return start;
}

std::uintptr_t alignedEnd(Region& region, std::size_t pageSize)
{
    return (region.address + region.size) & ~(pageSize - 1);
}

void clearRegionInfo(RegionInfo& regionInfo)
{
    regionInfo.start = 0;
    regionInfo.end = 0;
    regionInfo.alignedStart = 0;
    regionInfo.alignedEnd = 0;
    regionInfo.pageCount = 0;
    regionInfo.size = 0;
    regionInfo.alignedSize = 0;
    regionInfo.firstPage = nullptr;
    regionInfo.lastPage = nullptr;
}

void initRegionInfo(RegionInfo& regionInfo, Region& region, std::size_t pageSize)
{
    regionInfo.start = region.address;
    regionInfo.end = region.address + region.size;
    regionInfo.alignedStart = alignedStart(region, pageSize);
    regionInfo.alignedEnd = alignedEnd(region, pageSize);
    regionInfo.pageCount = (regionInfo.alignedEnd - regionInfo.alignedStart) / pageSize;
    regionInfo.size = region.size;
    regionInfo.alignedSize = regionInfo.pageCount * pageSize;
    regionInfo.firstPage = nullptr;
    regionInfo.lastPage = nullptr;
}

} // namespace Memory
