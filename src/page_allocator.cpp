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

#include "page_allocator.h"

#include <cassert>
#include <cmath>

namespace memory {

PageAllocator::PageAllocator() noexcept
{
    clear();
}

bool PageAllocator::init(Region* regions, std::size_t pageSize)
{
    assert(regions);

    clear();

    for (std::size_t i = 0; regions[i].size != 0; ++i) {
        if (i == MAX_REGIONS_COUNT)
            return false;

        RegionInfo regionInfo{};
        if (initRegionInfo(regionInfo, regions[i], pageSize))
            m_regionsInfo[m_validRegionsCount++] = regionInfo;
    }

    if (!(m_pagesCount = countPages()))
        return false;

    m_pageSize = pageSize;
    m_descRegionIdx = chooseDescRegion();
    m_pagesHead = reinterpret_cast<Page*>(m_regionsInfo[m_descRegionIdx].alignedStart);
    m_pagesTail = m_pagesHead + m_pagesCount - 1;

    auto* page = m_pagesHead;
    for (std::size_t i = 0; i < m_validRegionsCount; ++i) {
        auto& region = m_regionsInfo[i];

        region.firstPage = page;
        region.lastPage = page + region.pageCount - 1;

        for (auto addr = region.alignedStart; addr != region.alignedEnd; addr += m_pageSize) {
            page->init();
            page->setAddress(addr);
            page = page->nextSibling();
        }

        Page* group = region.firstPage;
        initGroup(group, region.pageCount);

        if (i == m_descRegionIdx) {
            m_descPagesCount = reserveDescPages();
            std::tie(std::ignore, group) = splitGroup(group, m_descPagesCount);
        }

        if (group)
            addGroup(group);
    }

    return true;
}

void PageAllocator::clear()
{
    for (auto& region : m_regionsInfo)
        clearRegionInfo(region);

    m_validRegionsCount = 0;
    m_pageSize = 0;
    m_descRegionIdx = 0;
    m_descPagesCount = 0;
    m_pagesHead = nullptr;
    m_pagesTail = nullptr;
    m_freeGroupLists.fill(nullptr);
    m_pagesCount = 0;
    m_freePagesCount = 0;
}

Page* PageAllocator::allocate(std::size_t count)
{
    if (m_freePagesCount < count || count == 0)
        return nullptr;

    std::size_t idx = groupIdx(count);
    for (auto i = idx; i < m_freeGroupLists.size(); ++i) {
        for (Page* group = m_freeGroupLists[i]; group != nullptr; group = group->next()) {
            if (group->groupSize() < count)
                continue;

            removeGroup(group);

            Page* allocatedGroup = nullptr;
            Page* remainingGroup = nullptr;
            std::tie(allocatedGroup, remainingGroup) = splitGroup(group, count);

            if (remainingGroup)
                addGroup(remainingGroup);

            return allocatedGroup;
        }
    }

    return nullptr;
}

void PageAllocator::release(Page* pages)
{
    if (!pages)
        return;

    // clang-format off
    Page* joinedGroup = pages;

    // Try joining with pages above the released group.
    do {
        Page* lastAbove = joinedGroup->prevSibling();
        if (!isValidPage(lastAbove))
            break;

        if (getRegion(joinedGroup->address()) != getRegion(lastAbove->address()))
            break;

        if (lastAbove->isUsed())
            break;

        Page* firstAbove = lastAbove - lastAbove->groupSize() + 1;
        removeGroup(firstAbove);
        joinedGroup = joinGroup(firstAbove, joinedGroup);
    }
    while (true);

    // Try joining with pages below the released group.
    do {
        Page* lastJoined = joinedGroup + joinedGroup->groupSize() - 1;
        Page* firstBelow = lastJoined->nextSibling();
        if (!isValidPage(firstBelow))
            break;

        if (getRegion(lastJoined->address()) != getRegion(firstBelow->address()))
            break;

        if (firstBelow->isUsed())
            break;

        removeGroup(firstBelow);
        joinedGroup = joinGroup(joinedGroup, firstBelow);
    }
    while (true);
    // clang-format on

    addGroup(joinedGroup);
}

std::size_t PageAllocator::countPages()
{
    std::size_t pagesCount = 0;
    for (auto& region : m_regionsInfo)
        pagesCount += region.pageCount;

    return pagesCount;
}

std::size_t PageAllocator::chooseDescRegion()
{
    std::size_t descAreaSize = m_pagesCount * sizeof(Page);

    std::size_t selectedIdx = 0;
    for (std::size_t i = 0; i < m_validRegionsCount; ++i) {
        if (m_regionsInfo[i].alignedSize < descAreaSize)
            continue;

        if (m_regionsInfo[i].alignedSize < m_regionsInfo[selectedIdx].alignedSize)
            selectedIdx = i;
    }

    return selectedIdx;
}

std::size_t PageAllocator::reserveDescPages()
{
    std::size_t reservedCount = 0;
    auto& descRegion = m_regionsInfo[m_descRegionIdx];
    for (auto* page = descRegion.firstPage; page <= m_pagesTail; page = page->nextSibling()) {
        if (page->address() >= reinterpret_cast<uintptr_t>(m_pagesTail->nextSibling()))
            break;

        page->setUsed(true);
        ++reservedCount;

        if (page == descRegion.lastPage)
            break;
    }

    return reservedCount;
}

bool PageAllocator::isValidPage(Page* page)
{
    return (page >= m_pagesHead && page <= m_pagesTail);
}

RegionInfo* PageAllocator::getRegion(std::uintptr_t addr)
{
    auto alignedAddr = addr & ~(m_pageSize - 1);

    for (std::size_t i = 0; i < m_validRegionsCount; ++i) {
        auto& region = m_regionsInfo[i];

        if (region.alignedStart <= alignedAddr && region.alignedEnd >= alignedAddr)
            return &region;
    }

    return nullptr;
}

Page* PageAllocator::getPage(std::uintptr_t addr)
{
    auto alignedAddr = addr & ~(m_pageSize - 1);

    RegionInfo* pageRegion = getRegion(alignedAddr);
    if (!pageRegion)
        return nullptr;

    for (auto* page = pageRegion->firstPage; page <= pageRegion->lastPage; page = page->nextSibling()) {
        if (page->address() == alignedAddr)
            return page;
    }

    assert(false);
}

PageAllocator::Stats PageAllocator::getStats()
{
    Stats stats{};
    stats.pageSize = m_pageSize;
    stats.pagesCount = m_pagesCount;
    stats.freePagesCount = m_freePagesCount;
    stats.descRegionIdx = m_descRegionIdx;
    stats.descPagesCount = m_descPagesCount;

    return stats;
}

std::size_t PageAllocator::groupIdx(std::size_t pageCount)
{
    if (pageCount < 2)
        return 0;

    return static_cast<std::size_t>(std::floor(std::log2(pageCount)) - 1);
}

void PageAllocator::initGroup(Page* group, std::size_t groupSize)
{
    assert(group);

    Page* firstPage = group;
    Page* lastPage = group + groupSize - 1;
    firstPage->setGroupSize(groupSize);
    lastPage->setGroupSize(groupSize);
}

void PageAllocator::clearGroup(Page* group)
{
    assert(group);

    Page* firstPage = group;
    Page* lastPage = group + group->groupSize() - 1;
    firstPage->setGroupSize(0);
    lastPage->setGroupSize(0);
}

void PageAllocator::addGroup(Page* group)
{
    assert(group);

    std::size_t idx = groupIdx(group->groupSize());
    group->addToList(&m_freeGroupLists[idx]);
    m_freePagesCount += group->groupSize();

    for (std::size_t i = 0; i < group->groupSize(); ++i) {
        auto* page = group + i;
        page->setUsed(false);
    }
}

void PageAllocator::removeGroup(Page* group)
{
    assert(group);

    std::size_t idx = groupIdx(group->groupSize());
    group->removeFromList(&m_freeGroupLists[idx]);
    m_freePagesCount -= group->groupSize();

    for (std::size_t i = 0; i < group->groupSize(); ++i) {
        auto* page = group + i;
        page->setUsed(true);
    }
}

std::tuple<Page*, Page*> PageAllocator::splitGroup(Page* group, std::size_t size)
{
    assert(group);
    assert(size);
    assert(size <= group->groupSize());

    if (size == group->groupSize())
        return std::tuple<Page*, Page*>(group, nullptr);

    std::size_t secondSize = group->groupSize() - size;
    clearGroup(group);

    Page* firstGroup = group;
    Page* secondGroup = group + size;

    initGroup(firstGroup, size);
    initGroup(secondGroup, secondSize);

    return std::make_tuple(firstGroup, secondGroup);
}

Page* PageAllocator::joinGroup(Page* firstGroup, Page* secondGroup)
{
    assert(firstGroup);
    assert(secondGroup);

    std::size_t joinedSize = firstGroup->groupSize() + secondGroup->groupSize();

    clearGroup(firstGroup);
    clearGroup(secondGroup);
    initGroup(firstGroup, joinedSize);

    return firstGroup;
}

} // namespace memory
