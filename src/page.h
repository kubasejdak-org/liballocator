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

#include "list_node.h"

#include <cstddef>
#include <cstdint>

namespace memory {

/// @class Page
/// @brief Represents a physical memory page.
class Page : public ListNode<Page> {
public:
    /// @brief Default constructor.
    /// @note This constructor is deleted, because pages should be initialized only in-place.
    Page() = delete;

    /// @brief Copy constructor.
    /// @param[in] other        Page to be used in initialization.
    /// @note This constructor is deleted, because pages should be initialized only in-place.
    Page(const Page& other) = delete;

    /// @brief Move constructor.
    /// @param[in] other        Page to be used in initialization.
    /// @note This constructor is deleted, because pages should be initialized only in-place.
    Page(Page&& other) = delete;

    /// @brief Initializes the page. It is used as a replacement for the constructor.
    void init();

    /// @brief Sets the physical address of the given page.
    /// @param[in] addr         Physical address to be set.
    void setAddress(std::uintptr_t addr);

    /// @brief Sets the size of the pages group, that this page represents.
    /// @param[in] groupSize    Size of the group to be set.
    /// @note Group size should be set only to the first and to the last page in the group.
    void setGroupSize(std::size_t groupSize);

    /// @brief Sets the 'used' flag of the current page to the given state.
    /// @param[in] value        State to be set.
    void setUsed(bool value);

    /// @brief Returns the page, that lies immediately after the given page.
    /// @return Pointer to the next sibling page.
    Page* nextSibling();

    /// @brief Returns the page, that lies immediately before the given page.
    /// @return Pointer to the previous sibling page.
    Page* prevSibling();

    /// @brief Returns physical address of the current page.
    /// @return Physical address.
    std::uintptr_t address();

    /// @brief Returns size of the group represented by the current page.
    /// @return Size of the group.
    std::size_t groupSize();

    /// @brief Returns flag indicating if current page is used or not.
    /// @return True if page is used, false otherwise.
    bool isUsed();

    /// @brief Checks if the Page class is naturally aligned.
    /// @return True if Page class is naturally aligned, false otherwise.
    /// @note Natural alignment of a class means, that its size is equal to the sum of all its data members.
    static constexpr bool isNaturallyAligned()
    {
        constexpr std::size_t requiredSize = sizeof(ListNode<Page>) // Inherited fields
                                           + sizeof(std::uintptr_t) // m_addr
                                           + sizeof(Page::Flags);   // m_flags
        return (requiredSize == sizeof(Page));
    }

private:
    /// @class Flags
    /// @brief Represents a packed set of flags used internally by pages.
    union Flags {
        struct {
            std::size_t groupSize : 21;     ///< Size of the group. This is set only for the first and to the last page in the group.
            bool used : 1;                  ///< Flag indicating whether this page is used or not.
        };

        std::uint32_t value;                ///< Raw bytes used to store the flags.
    };

private:
    std::uintptr_t m_addr;      ///< Physical address of the page.
    Flags m_flags;              ///< Flags of the page.
};

} // namespace memory

#endif
