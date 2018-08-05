/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2017-2018, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#ifndef PAGE_H
#define PAGE_H

#include <cstddef>
#include <cstdint>

namespace Memory {

class Page {
public:
    Page() = delete;
    Page(const Page& other) = delete;
    Page(Page&& other) = delete;

    void init();

    void addToList(Page** list);
    void removeFromList(Page** list);
    void setAddress(std::uintptr_t addr);
    void setGroupSize(std::size_t groupSize);
    void setUsed(bool value);

    Page* nextSibling();
    Page* prevSibling();
    Page* nextGroup();
    std::uintptr_t address();
    std::size_t groupSize();
    bool isUsed();

    static constexpr bool isNaturallyAligned()
    {
        constexpr std::size_t requiredSize = sizeof(Page*)          // m_nextGroup
                                           + sizeof(Page*)          // m_prevGroup
                                           + sizeof(std::uintptr_t) // m_addr
                                           + sizeof(Page::Flags);   // m_flags
        return (requiredSize == sizeof(Page));
    }

private:
    union Flags {
        struct {
            std::size_t groupSize : 21;
            bool used : 1;
        };

        std::uint32_t value;
    };

private:
    Page* m_nextGroup;
    Page* m_prevGroup;
    std::uintptr_t m_addr;
    Flags m_flags;
};

} // namespace Memory

#endif
