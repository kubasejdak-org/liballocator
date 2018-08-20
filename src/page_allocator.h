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

#ifndef PAGE_ALLOCATOR_H
#define PAGE_ALLOCATOR_H

#include "page.h"
#include "region_info.h"

#include <allocator/region.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <tuple>

namespace Memory {

/// @class PageAllocator
/// @brief Represents an allocator of physical pages.
class PageAllocator {
public:
    /// @class Stats
    /// @brief Represents the statistical data of the page allocator.
    struct Stats {
        std::size_t pageSize;           ///< Size of the page used by page allocator.
        std::size_t pagesCount;         ///< Total number of pages, that are known to the page allocator.
        std::size_t freePagesCount;     ///< Current number of the free pages in the page allocator.
        std::size_t descRegionIdx;      ///< Index of the memory region, that is used to store the page descriptors.
        std::size_t descPagesCount;     ///< Number of pages used to store the page descriptors.
    };

    /// @brief Default constructor.
    PageAllocator();

    /// @brief Initializes the page allocator with the given memory model.
    /// @param[in] regions          Array of memory regions to be used by page allocator. Last entry should be zeroed.
    /// @param[in] pageSize         Size of the page on the current platform.
    /// @return True on success, false otherwise.
    [[nodiscard]] bool init(Region* regions, std::size_t pageSize);

    /// @brief Clears the internal state of the page allocator.
    void clear();

    /// @brief Allocates the given number of physical pages.
    /// @param[in] count            Number of pages to be allocated.
    /// @note All allocated pages must be from the same region.
    [[nodiscard]] Page* allocate(std::size_t count);

    /// @brief Releases the given set of pages.
    /// @param[in] pages            List of pages to be released.
    void release(Page* pages);

private:
    /// @brief Returns the total number of pages from all known regions.
    /// @return Number of all pages from all known regions.
    std::size_t countPages();

    /// @brief Returns the index of the best region to store the page descriptors.
    /// @return Index of the region, where page descriptors will be stored.
    std::size_t chooseDescRegion();

    /// @brief Reserves the necessary number of pages to store the page descriptors.
    /// @return Number of pages, that are used to store the page descriptors.
    std::size_t reserveDescPages();

    /// @brief Checks if the given page is valid.
    /// @param[in] page             Page to be checked.
    /// @return True if page is valid, false otherwise.
    bool isValidPage(Page* page);
    RegionInfo* getRegion(std::uintptr_t addr);
    Page* getPage(std::uintptr_t addr);
    Stats getStats();

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
    std::size_t m_pageSize;
    std::size_t m_descRegionIdx;
    std::size_t m_descPagesCount;
    Page* m_pagesHead;
    Page* m_pagesTail;
    std::array<Page*, MAX_GROUP_IDX> m_freeGroupLists;
    std::size_t m_pagesCount;
    std::size_t m_freePagesCount;
};

} // namespace Memory

#endif
