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

#ifndef LIST_NODE_H
#define LIST_NODE_H

#include <cassert>

namespace memory {

/// @class ListNode
/// @brief Represents a node of the generic doubly-linked list.
template <typename T>
class ListNode {
public:
    /// @brief Default constructor.
    ListNode()
    {
        initListNode();
    }

    /// @brief Initializes the node.
    void initListNode()
    {
        m_next = nullptr;
        m_prev = nullptr;
    }

    /// @brief Returns the node from the list.
    /// @return Pointer to the next list node.
    T* next()
    {
        return m_next;
    }

    /// @brief Adds current node to the given list.
    /// @param[in,out] list     Pointer to the list, to which node should be added.
    void addToList(T** list)
    {
        assert(list);
        assert(!m_next);
        assert(!m_prev);

        if (*list) {
            m_next = *list;
            m_next->m_prev = static_cast<T*>(this);
        }

        *list = static_cast<T*>(this);
    }

    /// @brief Removes current node from the given list.
    /// @param[in,out] list     Pointer to the list, from which node should be removed.
    void removeFromList(T** list)
    {
        assert(list);
        assert(this == *list || m_next || m_prev);

        if (m_next)
            m_next->m_prev = m_prev;

        if (m_prev)
            m_prev->m_next = m_next;

        if (*list == this)
            *list = m_next;

        m_next = nullptr;
        m_prev = nullptr;
    }

private:
    T* m_next;    ///< Next node in the list.
    T* m_prev;    ///< Previous node in the list.
};

} // namespace memory

#endif
