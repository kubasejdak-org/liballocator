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

bool PageAllocator::init(Region *regions)
{
    for (int i = 0; regions[i].size != 0; ++i) {
        auto* start = alignedStart(regions[i]);
        auto* end = alignedEnd(regions[i]);

        // No use from regions with size smaller than one page.
        if (end - start < PAGE_SIZE)
            continue;

        Page* chain = nullptr;
        std::size_t chainSize = 0;

        for (auto* addr = start; (addr + PAGE_SIZE) <= end; addr += PAGE_SIZE) {
            auto* page = reinterpret_cast<Page*>(addr);
            page->init();

            if (!chain)
                chain = page;

            chain->attachPages(page);

            ++chainSize;
            ++m_allPagesCount;
            ++m_freePagesCount;
        }

        auto idx = chainIdx(chainSize);
        if (!m_freePages[idx])
            m_freePages[idx] = chain;

        m_freePages[idx]->addChain(chain);
    }

    return (m_allPagesCount != 0);
}

void PageAllocator::clear()
{
    m_allPagesCount = 0;
    m_freePagesCount = 0;
    m_freePages.fill(nullptr);
}

Page* PageAllocator::allocate(size_t count)
{
    if (m_freePagesCount < count)
        return nullptr;

    for (auto i = chainIdx(count); i < m_freePages.size(); ++i) {
        if (!m_freePages[i])
            continue;

        for (auto* chain = m_freePages[i]; chain != nullptr; chain = chain->nextChain()) {
            if (chain->pagesCount() < count)
                continue;

            m_freePages[i]->removeChain(chain);

            Page* remaining = nullptr;
            Page* allocated = nullptr;
            std::tie(remaining, allocated) = chain->detachPages(count);;

            m_freePages[i]->addChain(remaining);
            m_freePagesCount -= count;
            return allocated;
        }
    }

    return nullptr;
}
void PageAllocator::release(Page* pages)
{
}

std::size_t PageAllocator::chainIdx(size_t size)
{
    return static_cast<std::size_t>(ceil(log2(size)));
}

char* PageAllocator::alignedStart(Region& region)
{
    auto addr = reinterpret_cast<std::size_t>(region.address);
    auto start = addr & ~(PAGE_SIZE - 1);

    if (start < addr)
        start += PAGE_SIZE;

    return reinterpret_cast<char*>(start);
}

char* PageAllocator::alignedEnd(Region& region)
{
    auto addr = reinterpret_cast<std::size_t>(region.address);
    auto end = (addr + region.size) & ~(PAGE_SIZE - 1);

    return reinterpret_cast<char*>(end);
}

} // namespace Memory
