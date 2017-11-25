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

namespace Memory {

PageAllocator::PageAllocator() noexcept
{
    clear();
}

bool PageAllocator::init(Region* regions, std::size_t pageSize)
{
    m_pageSize = pageSize;
    Page* prevPage = m_freePages;

    for (int i = 0; regions[i].size != 0; ++i) {
        auto* start = alignedStart(regions[i]);
        auto* end = alignedEnd(regions[i]);

        // No use from regions with size smaller than one page.
        if (end - start < m_pageSize)
            continue;

        for (auto* addr = start; (addr + m_pageSize) <= end; addr += m_pageSize) {
            auto* page = reinterpret_cast<Page*>(addr);

            linkPages(page, prevPage);

            prevPage = page;
            ++m_allPagesCount;
            ++m_freePagesCount;

            if (!m_freePages)
                m_freePages = page;
        }
    }

    return (m_allPagesCount != 0);
}

void PageAllocator::clear() noexcept
{
    m_pageSize = 0;
    m_allPagesCount = 0;
    m_freePagesCount = 0;
    m_freePages = nullptr;
}

Page* PageAllocator::allocate()
{
    Page* page = m_freePages;
    m_freePages = page->next;

    unlinkPages(page, page->next);
    return page;
}

void PageAllocator::release(Page& page)
{
}

char* PageAllocator::alignedStart(Region& region)
{
    auto addr = reinterpret_cast<std::size_t>(region.address);
    auto start = addr & ~(m_pageSize - 1);

    if (start < addr)
        start += m_pageSize;

    return reinterpret_cast<char*>(start);
}

char* PageAllocator::alignedEnd(Region& region)
{
    auto addr = reinterpret_cast<std::size_t>(region.address);
    auto end = (addr + region.size) & ~(m_pageSize - 1);

    return reinterpret_cast<char*>(end);
}

void PageAllocator::linkPages(Page* page, Page* prev)
{
    page->prev = prev;
    page->next = nullptr;

    if (prev)
        prev->next = page;
}

void PageAllocator::unlinkPages(Page* page, Page* next)
{
    next->prev = page->prev;

    if (page->prev)
        page->prev->next = next;

    page->prev = nullptr;
    page->next = nullptr;
}

} // namespace Memory
