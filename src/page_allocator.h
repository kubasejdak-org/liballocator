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

#ifndef PAGE_ALLOCATOR_H
#define PAGE_ALLOCATOR_H

#include "page.h"
#include "region_info.h"

#include <zone_allocator/region.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>

namespace Memory {

class PageAllocator {
public:
    PageAllocator();

    [[nodiscard]] bool init(Region* regions, std::size_t pageSize);
    void clear();

    [[nodiscard]] Page* allocate(std::size_t count);
    void release(Page* pages);

private:
    std::size_t countPages();
    std::size_t chooseDescRegion();
    std::size_t reserveDescPages();
    Page* getPage(std::uintptr_t addr);

    std::size_t groupIdx(std::size_t pageCount);
    void initGroup(Page* group, std::size_t groupSize);
    void clearGroup(Page* group);
    void addGroup(Page* group);
    void removeGroup(Page* group);
    std::tuple<Page*, Page*> splitGroup(Page* group, std::size_t size);
    Page* joinGroup(Page* firstGroup, Page* secondGroup);

private:
    static constexpr int MAX_REGIONS_COUNT = 8;
    static constexpr int MAX_GROUP_IDX = 20;

private:
    std::array<RegionInfo, MAX_REGIONS_COUNT> m_regionsInfo;
    std::size_t m_validRegionsCount;
    Page* m_pagesHead;
    Page* m_pagesTail;
    std::array<Page*, MAX_GROUP_IDX> m_freeGroupLists;
    std::size_t m_pagesCount;
    std::size_t m_freePagesCount;
};

} // namespace Memory

#endif
