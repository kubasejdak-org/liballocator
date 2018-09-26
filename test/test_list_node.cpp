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

#include <catch2/catch.hpp>

#include <cstddef>

// Make access to private members for testing.
// clang-format off
#define private     public
// clang-format on

#include <list_node.h>

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace memory;

struct TestNode : public ListNode<TestNode> {
public:
    TestNode() = default;
    int value{0};
};

TEST_CASE("List node is properly initialized", "[list_node]")
{
    TestNode node;
    node.initListNode();
    REQUIRE(node.m_next == nullptr);
    REQUIRE(node.m_prev == nullptr);
}

TEST_CASE("Adding to empty list", "[list_node]")
{
    TestNode node;
    node.initListNode();
    node.value = 1;

    TestNode* list = nullptr;
    node.addToList(&list);
    REQUIRE(list == &node);
    REQUIRE(list->value == 1);
    REQUIRE(node.m_next == nullptr);
    REQUIRE(node.m_prev == nullptr);
}

TEST_CASE("Adding to non-empty list", "[list_node]")
{
    constexpr int nodeCount = 5;
    TestNode node[nodeCount];

    TestNode* list = nullptr;
    for (int i = 0; i < nodeCount; ++i) {
        node[i].initListNode();
        node[i].value = i;
        node[i].addToList(&list);
    }

    SECTION("All nodes are in the list")
    {
        bool nodePresent[nodeCount] = {};

        for (auto* it = list; it != nullptr; it = it->next()) {
            auto idx = it->value;
            REQUIRE(idx < nodeCount);
            nodePresent[idx] = true;
        }

        for (auto& present : nodePresent)
            REQUIRE(present);
    }

    SECTION("All nodes are in correct order")
    {
        int idx = nodeCount - 1;
        for (auto* it = list; it != nullptr; it = it->next(), --idx)
            REQUIRE(it->value == node[idx].value);
    }
}

TEST_CASE("Removing from list with 5 nodes", "[list_node]")
{
    constexpr int nodeCount = 5;
    TestNode node[nodeCount];

    TestNode* list = nullptr;
    for (int i = 0; i < nodeCount; ++i) {
        node[i].initListNode();
        node[i].value = i;
        node[i].addToList(&list);
    }

    SECTION("Removing first node from list")
    {
        int idx = 4;
        node[idx].removeFromList(&list);
        REQUIRE(node[idx].value == idx);
        REQUIRE(node[idx].m_next == nullptr);
        REQUIRE(node[idx].m_prev == nullptr);

        int i = nodeCount - 1;
        for (auto* it = list; it != nullptr; it = it->next(), --i) {
            if (i == idx)
                --i;

            REQUIRE(it->value == node[i].value);
        }
    }

    SECTION("Removing middle node from list")
    {
        int idx = 2;
        node[idx].removeFromList(&list);
        REQUIRE(node[idx].value == idx);
        REQUIRE(node[idx].m_next == nullptr);
        REQUIRE(node[idx].m_prev == nullptr);

        int i = nodeCount - 1;
        for (auto* it = list; it != nullptr; it = it->next(), --i) {
            if (i == idx)
                --i;

            REQUIRE(it->value == node[i].value);
        }
    }

    SECTION("Removing last node from list")
    {
        int idx = 0;
        node[idx].removeFromList(&list);
        REQUIRE(node[idx].value == idx);
        REQUIRE(node[idx].m_next == nullptr);
        REQUIRE(node[idx].m_prev == nullptr);

        int i = nodeCount - 1;
        for (auto* it = list; it != nullptr; it = it->next(), --i) {
            if (i == idx)
                --i;

            REQUIRE(it->value == node[i].value);
        }
    }

    SECTION("Removing all nodes starting from first")
    {
        for (int i = nodeCount - 1; i >= 0; --i) {
            node[i].removeFromList(&list);
            REQUIRE(node[i].value == i);
            REQUIRE(node[i].m_next == nullptr);
            REQUIRE(node[i].m_prev == nullptr);
        }

        REQUIRE(list == nullptr);
    }

    SECTION("Removing all nodes starting from last")
    {
        for (int i = 0; i < nodeCount; ++i) {
            node[i].removeFromList(&list);
            REQUIRE(node[i].value == i);
            REQUIRE(node[i].m_next == nullptr);
            REQUIRE(node[i].m_prev == nullptr);
        }

        REQUIRE(list == nullptr);
    }
}

TEST_CASE("Removing from list with 1 node", "[list_node]")
{
    TestNode node;
    node.initListNode();
    node.value = 1;

    TestNode* list = nullptr;
    node.addToList(&list);

    node.removeFromList(&list);
    REQUIRE(list == nullptr);
    REQUIRE(node.value == 1);
    REQUIRE(node.m_next == nullptr);
    REQUIRE(node.m_prev == nullptr);
}
