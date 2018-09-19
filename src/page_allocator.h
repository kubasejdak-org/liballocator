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

namespace memory {

/// @class PageAllocator
/// @brief Represents an allocator of physical pages.
class PageAllocator {
public:
    /// @class Stats
    /// @brief Represents the statistical data of the page allocator.
    struct Stats {
        std::size_t totalMemorySize;        ///< Total size of the memory passed during initialization.
        std::size_t effectiveMemorySize;    ///< Effective size of the memory, that can be used by the allocator.
        std::size_t userMemorySize;         ///< Total size of the memory available to the user.
        std::size_t freeMemorySize;         ///< Size of the remaining user memory.
        std::size_t pageSize;               ///< Size of the page used by the allocator.
        std::size_t totalPagesCount;        ///< Total number of the pages known to the allocator.
        std::size_t reservedPagesCount;     ///< Number of pages reserved for the allocator.
        std::size_t freePagesCount;         ///< Current number of the free pages.
    };

    /// @brief Default constructor.
    PageAllocator() noexcept;
    /// @brief Initializes the page allocator with the given memory model.
    /// @param[in] regions          Array of memory regions to be used by page allocator. Last entry should be zeroed.
    /// @param[in] pageSize         Size of the page on the current platform.
    /// @return True on success, false otherwise.
    [[nodiscard]] bool init(Region* regions, std::size_t pageSize);

    /// @brief Clears the internal state of the page allocator.
    void clear();

    /// @brief Allocates the given number of physical pages.
    /// @param[in] count            Number of pages to be allocated.
    /// @return A set of allocated pages.
    /// @note All allocated pages must be from the same region.
    [[nodiscard]] Page* allocate(std::size_t count);

    /// @brief Releases the given set of pages.
    /// @param[in] pages            List of pages to be released.
    void release(Page* pages);

    /// @brief Returns the Page, which contains the given address.
    /// @param[in] addr             Address for which Page should be found.
    /// @return Pointer to Page containing given address if found, nullptr otherwise.
    Page* getPage(std::uintptr_t addr);

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

    /// @brief Returns the RegionInfo, which contains the given address.
    /// @param[in] addr             Address for which RegionInfo should be found.
    /// @return Pointer to RegionInfo containing given address if found, nullptr otherwise.
    RegionInfo* getRegion(std::uintptr_t addr);

    /// @brief Returns the current statistics of PageAllocator.
    /// @return PageAllocator statistics.
    Stats getStats();

    /// @brief Calculates index in the groups array, for which group with the given page count should be stored.
    /// @param[in] pageCount        Number of pages, for which index should be calculated.
    /// @return Index in the groups array.
    std::size_t groupIdx(std::size_t pageCount);

    /// @brief Initializes given group.
    /// @param[in,out] group        Group to be initialized.
    /// @param[in] groupSize        Size of the initialized group.
    void initGroup(Page* group, std::size_t groupSize);

    /// @brief Clears the given group.
    /// @param[in,out] group        Group to be cleared.
    void clearGroup(Page* group);

    /// @brief Adds given group to the array of free groups.
    /// @param[in,out] group        Group to be added.
    void addGroup(Page* group);

    /// @brief Removes given group from the array of free groups.
    /// @param[in,out] group        Group to be removed.
    void removeGroup(Page* group);

    /// @brief Splits given group into one of given size and second with the remaining size.
    /// @param[in] group            Group to be splitted.
    /// @param[in] size             Target size of the first group.
    /// @returns Tuple with group of demanded size and with the group of the remaining size.
    /// @note If the given group has already the correct size, then second pointer in the tuple is nullptr.
    std::tuple<Page*, Page*> splitGroup(Page* group, std::size_t size);

    /// @brief Joins two given groups into one.
    /// @param[in] firstGroup       First group to be joined.
    /// @param[in] secondGroup      Second group to be joined.
    /// @return Group that is a sum of the two given groups.
    Page* joinGroup(Page* firstGroup, Page* secondGroup);

private:
    static constexpr int MAX_REGIONS_COUNT = 8;                 ///< Maximal supported number of memory regions.
    static constexpr int MAX_GROUP_IDX = 20;                    ///< Maximal index of the group in the free array.

private:
    std::array<RegionInfo, MAX_REGIONS_COUNT> m_regionsInfo;    ///< Array describing all known regions.
    std::size_t m_validRegionsCount;                            ///< Number of used regions.
    std::size_t m_pageSize;                                     ///< Size of the page used on this platform.
    std::size_t m_descRegionIdx;                                ///< Index of the region used to store page descriptors.
    std::size_t m_descPagesCount;                               ///< Number of pages used to store page descriptors.
    Page* m_pagesHead;                                          ///< Head of the page descriptors list.
    Page* m_pagesTail;                                          ///< Tail of the page descriptors list.
    std::array<Page*, MAX_GROUP_IDX> m_freeGroupLists;          ///< Array of the groups with free pages.
    std::size_t m_pagesCount;                                   ///< Total number of pages known to the PageAllocator.
    std::size_t m_freePagesCount;                               ///< Current number of free pages.
};

} // namespace memory

#endif
