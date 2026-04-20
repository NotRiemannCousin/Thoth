#include <gtest/gtest.h>
#include <Thoth/Dsa/LinearMap.hpp>

#include <string>

using Thoth::Dsa::LinearMap;
using IntMap    = LinearMap<int, std::string>;
using StringMap = LinearMap<std::string, int>;


#pragma region Construction

struct LinearMapConstructTest : testing::Test {};

TEST_F(LinearMapConstructTest, DefaultConstruct_IsEmpty) {
    IntMap m;
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(m.size(), 0u);
}

TEST_F(LinearMapConstructTest, InitializerList_InsertsSorted) {
    IntMap m{{ {3, "c"}, {1, "a"}, {2, "b"} }};
    EXPECT_EQ(m.size(), 3u);
    // Verify sorted order: key 1 < 2 < 3
    auto it{ m.begin() };
    EXPECT_EQ(it->first, 1); ++it;
    EXPECT_EQ(it->first, 2); ++it;
    EXPECT_EQ(it->first, 3);
}

TEST_F(LinearMapConstructTest, InitializerList_DeduplicatesKeys) {
    // Duplicate key 1: only first one (or last one wins – implementation defined, but size must be 1)
    IntMap m{{ {1, "first"}, {1, "second"} }};
    EXPECT_EQ(m.size(), 1u);
}

TEST_F(LinearMapConstructTest, CopyConstruct_EqualToOriginal) {
    IntMap orig{{ {1, "a"}, {2, "b"} }};
    const IntMap copy{ orig };
    EXPECT_EQ(copy, orig);
}

TEST_F(LinearMapConstructTest, MoveConstruct_OriginalEmptied) {
    IntMap orig{{ {1, "a"}, {2, "b"} }};
    const IntMap moved{ std::move(orig) };
    EXPECT_EQ(moved.size(), 2u);
}

TEST_F(LinearMapConstructTest, CopyAssign_EqualToOriginal) {
    IntMap orig{{ {5, "five"} }};
    IntMap copy;
    copy = orig;
    EXPECT_EQ(copy, orig);
}

TEST_F(LinearMapConstructTest, MoveAssign_Succeeds) {
    IntMap orig{{ {7, "seven"} }};
    IntMap target;
    target = std::move(orig);
    EXPECT_TRUE(target.contains(7));
}

#pragma endregion


#pragma region try_emplace / insert_or_assign

struct LinearMapInsertTest : testing::Test {
    IntMap m{{ {1, "one"}, {3, "three"} }};
};

TEST_F(LinearMapInsertTest, TryEmplace_NewKey_InsertedAndReturnsTrue) {
    const auto [it, inserted]{ m.try_emplace(2, "two") };
    EXPECT_TRUE(inserted);
    EXPECT_EQ(it->first, 2);
    EXPECT_EQ(it->second, "two");
    EXPECT_EQ(m.size(), 3u);
}

TEST_F(LinearMapInsertTest, TryEmplace_ExistingKey_NotInsertedReturnsFalse) {
    const auto [it, inserted]{ m.try_emplace(1, "other") };
    EXPECT_FALSE(inserted);
    // Value must not have changed
    EXPECT_EQ(it->second, "one");
    EXPECT_EQ(m.size(), 2u);
}

TEST_F(LinearMapInsertTest, InsertOrAssign_NewKey_InsertedReturnsTrue) {
    const auto [it, inserted]{ m.insert_or_assign(5, std::string("five")) };
    EXPECT_TRUE(inserted);
    EXPECT_EQ(m.size(), 3u);
}

TEST_F(LinearMapInsertTest, InsertOrAssign_ExistingKey_UpdatedReturnsFalse) {
    const auto [it, inserted]{ m.insert_or_assign(1, std::string("ONE")) };
    EXPECT_FALSE(inserted);
    EXPECT_EQ(it->second, "ONE");
}

TEST_F(LinearMapInsertTest, InsertOrAssign_MaintainsSortedOrder) {
    m.insert_or_assign(2, std::string("two"));
    m.insert_or_assign(0, std::string("zero"));
    int prev{ -1 };
    for (const auto& [k, v] : m) {
        EXPECT_GT(k, prev);
        prev = k;
    }
}

TEST_F(LinearMapInsertTest, OperatorBracket_NewKey_DefaultInitialized) {
    m[10];
    EXPECT_TRUE(m.contains(10));
    EXPECT_EQ(m[10], "");  // default-constructed std::string
}

TEST_F(LinearMapInsertTest, OperatorBracket_ExistingKey_ReturnsRef) {
    m[1] = "updated";
    EXPECT_EQ(m[1], "updated");
}

#pragma endregion


#pragma region find / exists / contains

struct LinearMapFindTest : testing::Test {
    IntMap m{{ {1, "one"}, {2, "two"}, {3, "three"} }};
};

TEST_F(LinearMapFindTest, Find_ExistingKey_ReturnsIterator) {
    const auto it{ m.find(2) };
    ASSERT_NE(it, m.end());
    EXPECT_EQ(it->second, "two");
}

TEST_F(LinearMapFindTest, Find_MissingKey_ReturnsEnd) {
    EXPECT_EQ(m.find(99), m.end());
}

TEST_F(LinearMapFindTest, Find_Const_ExistingKey) {
    const IntMap& cm{ m };
    const auto it{ cm.find(3) };
    ASSERT_NE(it, cm.end());
    EXPECT_EQ(it->second, "three");
}

TEST_F(LinearMapFindTest, Exists_PresentKey_True) {
    EXPECT_TRUE(m.exists(1));
}

TEST_F(LinearMapFindTest, Exists_MissingKey_False) {
    EXPECT_FALSE(m.exists(5));
}

TEST_F(LinearMapFindTest, Contains_PresentKey_True) {
    EXPECT_TRUE(m.contains(2));
}

TEST_F(LinearMapFindTest, Contains_MissingKey_False) {
    EXPECT_FALSE(m.contains(100));
}

TEST_F(LinearMapFindTest, Find_StringKey_HeterogeneousLookup) {
    StringMap sm{{ {"alpha", 1}, {"beta", 2} }};
    const auto it{ sm.find(std::string_view{"alpha"}) };
    ASSERT_NE(it, sm.end());
    EXPECT_EQ(it->second, 1);
}

#pragma endregion


#pragma region erase

struct LinearMapEraseTest : testing::Test {
    IntMap m{{ {1, "one"}, {2, "two"}, {3, "three"} }};
};

TEST_F(LinearMapEraseTest, EraseByKey_ExistingKey_ReturnsTrue) {
    EXPECT_TRUE(m.erase(2));
    EXPECT_EQ(m.size(), 2u);
    EXPECT_FALSE(m.contains(2));
}

TEST_F(LinearMapEraseTest, EraseByKey_MissingKey_ReturnsFalse) {
    EXPECT_FALSE(m.erase(99));
    EXPECT_EQ(m.size(), 3u);
}

TEST_F(LinearMapEraseTest, EraseByIterator_RemovesElement) {
    const auto it{ m.find(1) };
    ASSERT_NE(it, m.end());
    m.erase(it);
    EXPECT_FALSE(m.contains(1));
    EXPECT_EQ(m.size(), 2u);
}

TEST_F(LinearMapEraseTest, EraseAll_ByKey_EmptiesMap) {
    m.erase(1);
    m.erase(2);
    m.erase(3);
    EXPECT_TRUE(m.empty());
}

TEST_F(LinearMapEraseTest, Clear_EmptiesMap) {
    m.clear();
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(m.size(), 0u);
}

#pragma endregion


#pragma region Iterators

struct LinearMapIteratorTest : testing::Test {
    IntMap m{{ {10, "ten"}, {20, "twenty"}, {30, "thirty"} }};
};

TEST_F(LinearMapIteratorTest, BeginEnd_CountMatchesSize) {
    std::size_t count{};
    for (const auto& pair : m) { (void)pair; count++; }
    EXPECT_EQ(count, m.size());
}

TEST_F(LinearMapIteratorTest, Iteration_SortedAscending) {
    int prev{ -1 };
    for (const auto& [k, v] : m) {
        EXPECT_GT(k, prev);
        prev = k;
    }
}

TEST_F(LinearMapIteratorTest, ConstIteration_Works) {
    const IntMap& cm{ m };
    std::size_t count{};
    for (const auto& pair : cm) { (void)pair; count++; }
    EXPECT_EQ(count, cm.size());
}

TEST_F(LinearMapIteratorTest, CbeginCend_Work) {
    std::size_t count{};
    for (auto it{ m.cbegin() }; it != m.cend(); ++it) count++;
    EXPECT_EQ(count, m.size());
}

#pragma endregion


#pragma region Equality

struct LinearMapEqualityTest : testing::Test {};

TEST_F(LinearMapEqualityTest, Equality_SameContents_Equal) {
    IntMap a{{ {1, "a"}, {2, "b"} }};
    IntMap b{{ {1, "a"}, {2, "b"} }};
    EXPECT_EQ(a, b);
}

TEST_F(LinearMapEqualityTest, Equality_DifferentValues_NotEqual) {
    IntMap a{{ {1, "a"} }};
    IntMap b{{ {1, "b"} }};
    EXPECT_NE(a, b);
}

TEST_F(LinearMapEqualityTest, Equality_DifferentKeys_NotEqual) {
    IntMap a{{ {1, "a"} }};
    IntMap b{{ {2, "a"} }};
    EXPECT_NE(a, b);
}

TEST_F(LinearMapEqualityTest, Equality_DifferentSizes_NotEqual) {
    IntMap a{{ {1, "a"}, {2, "b"} }};
    IntMap b{{ {1, "a"} }};
    EXPECT_NE(a, b);
}

TEST_F(LinearMapEqualityTest, Equality_EmptyMaps_Equal) {
    EXPECT_EQ(IntMap{}, IntMap{});
}

#pragma endregion


#pragma region Stress: many insertions stay sorted

TEST(LinearMapStressTest, ManyInsertions_RemainSorted) {
    IntMap m;
    constexpr int N{ 200 };
    // Insert in reverse order
    for (int i{ N }; i >= 1; --i)
        m.insert_or_assign(i, std::to_string(i));

    EXPECT_EQ(m.size(), static_cast<std::size_t>(N));

    int prev{ 0 };
    for (const auto& [k, v] : m) {
        EXPECT_EQ(k, prev + 1);
        prev = k;
    }
}

TEST(LinearMapStressTest, FindAfterManyInsertions) {
    IntMap m;
    for (int i{}; i < 100; ++i)
        m.try_emplace(i, std::to_string(i));

    for (int i{}; i < 100; ++i) {
        const auto it{ m.find(i) };
        ASSERT_NE(it, m.end());
        EXPECT_EQ(it->second, std::to_string(i));
    }
}

#pragma endregion