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

#include <cmath>

namespace Memory {

PageAllocator::PageAllocator()
{
    clear();
}

bool PageAllocator::init(Region* regions, std::size_t pageSize)
{
    for (int i = 0; i < regions[i].size != 0; ++i)
        initRegionInfo(m_regionsInfo[i], regions[i], pageSize);

    if (!countPages())
        return false;

    auto descRegionIdx = chooseDescRegion();
    m_pagesHead = reinterpret_cast<Page*>(m_regionsInfo[descRegionIdx].start);
    m_pagesTail = m_pagesHead + m_pagesCount - 1;

    auto* page = m_pagesHead;
    for (auto& region : m_regionsInfo) {
        for (auto addr = region.alignedStart; addr != region.alignedEnd; addr += pageSize) {
            page->init();
            page->setAddress(addr);
            page = page->nextSibling();
        }
    }

    reserveDescPages();
    return true;
}

void PageAllocator::clear()
{
    for (auto& region : m_regionsInfo)
        clearRegionInfo(region);

    m_pagesHead = nullptr;
    m_pagesTail = nullptr;
    m_freeGroups.fill(nullptr);
    m_pagesCount = 0;
}

std::size_t PageAllocator::countPages()
{
    for (auto& region : m_regionsInfo)
        m_pagesCount += region.pageCount;

    return m_pagesCount;
}

int PageAllocator::chooseDescRegion()
{
    size_t descAreaSize = m_pagesCount * sizeof(Page);

    int selectedIdx = 0;
    for (int i = 0; i < m_regionsInfo.size(); ++i) {
        if (m_regionsInfo[i].size < descAreaSize)
            continue;

        if (m_regionsInfo[i].size < m_regionsInfo[selectedIdx].size)
            selectedIdx = i;
    }

    return selectedIdx;
}

void PageAllocator::reserveDescPages()
{
    for (auto* page = m_pagesHead; page->address() <= m_pagesTail->address(); page = page->nextSibling())
        page->setUsed(true);
}

int PageAllocator::groupIdx(int pageCount)
{
    return static_cast<int>(ceil(log2(pageCount)) - 1);
}

} // namespace Memory
