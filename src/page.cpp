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

#include "page.h"

#include <cassert>

namespace Memory {

void Page::init()
{
    m_nextPage = nullptr;
    m_nextChain = nullptr;
}

void Page::attachPages(Page* page)
{
    if (page == this)
        return;

    Page* last = nullptr;
    for (last = this; last->m_nextPage != nullptr; last = last->m_nextPage);

    last->m_nextPage = page;
}

std::tuple<Page*, Page*> Page::detachPages(std::size_t count)
{
    assert(count <= pagesCount());

    auto remainingSize = pagesCount() - count;
    if (remainingSize == 0)
        return std::make_tuple(reinterpret_cast<Page*>(NULL), this);

    Page* it = this;
    for (int i = 1; i < remainingSize; ++i, it = it->m_nextPage);

    auto* remaining = it;
    auto* detached = it->m_nextPage;

    remaining->m_nextPage = nullptr;
    return std::make_tuple(remaining, detached);
}

std::size_t Page::pagesCount()
{
    size_t size = 1;
    for (auto* it = this; it->m_nextPage != nullptr; it = it->m_nextPage, ++size);

    return size;
}

Page* Page::nextPage()
{
    return m_nextPage;
}

void Page::addChain(Page* chain)
{
    if (chain == this)
        return;

    Page* last = nullptr;
    for (last = this; last->m_nextChain != nullptr; last = last->m_nextChain);

    last->m_nextChain = chain;
}

void Page::removeChain(Page* chain)
{
    if (chain == this)
        return;

    Page* it = nullptr;
    for (it = this; it != nullptr && it->m_nextChain != chain; it = it->m_nextChain);

    assert(it);
    it->m_nextChain = chain->m_nextChain;
}

Page* Page::nextChain()
{
    return m_nextChain;
}

} // namespace Memory
