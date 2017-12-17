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

#ifndef PAGE_H
#define PAGE_H

#include <cstdint>

namespace Memory {

class Page {
public:
    Page() = delete;
    Page(const Page& other) = delete;
    Page(Page&& other) = delete;

    void init();

    void setNext(Page* next);
    void setPrev(Page* prev);
    void addToGroup(Page** group);
    void removeFromGroup(Page** group);
    void setAddress(std::uintptr_t addr);
    void setUsed(bool value);

    Page* prevSibling();
    Page* nextSibling();
    Page* next();
    Page* prev();
    Page* nextGroup();
    Page* prevGroup();
    std::size_t groupSize();
    std::uintptr_t address();
    bool isUsed();

private:
    union Flags {
        struct {
            bool used : 1;
        };

        std::uint32_t value;
    };

private:
    Page* m_next;
    Page* m_prev;
    Page* m_nextGroup;
    Page* m_prevGroup;
    std::size_t m_groupSize;
    std::uintptr_t m_addr;
    Flags m_flags;
} __attribute__((packed));

} // namespace Memory

#endif
