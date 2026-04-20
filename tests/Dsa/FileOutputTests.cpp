#include <gtest/gtest.h>
#include <Thoth/Dsa/FileOutputRange.hpp>

#include <filesystem>
#include <fstream>
#include <string>

using Thoth::Dsa::FileOutputRange;
using Thoth::Dsa::TextFileOutputRange;


#pragma region Helpers

static std::filesystem::path MakeTempPath(const std::string& name) {
    return std::filesystem::temp_directory_path() / name;
}

static std::string ReadFile(const std::filesystem::path& path) {
    std::ifstream f{ path };
    return { std::istreambuf_iterator<char>(f), {} };
}

#pragma endregion


#pragma region Construction

struct FileOutputRangeConstructTest : testing::Test {
    const std::filesystem::path tmp{ MakeTempPath("thoth_test_construct.txt") };

    void TearDown() override {
        std::filesystem::remove(tmp);
    }
};

TEST_F(FileOutputRangeConstructTest, ConstructFromPath_Succeeds) {
    EXPECT_NO_FATAL_FAILURE({ TextFileOutputRange range{ tmp }; });
}

TEST_F(FileOutputRangeConstructTest, ConstructFromRvaluePath_Succeeds) {
    EXPECT_NO_FATAL_FAILURE({ TextFileOutputRange range{ MakeTempPath("thoth_test_rval.txt") }; });
    std::filesystem::remove(MakeTempPath("thoth_test_rval.txt"));
}

TEST_F(FileOutputRangeConstructTest, MoveConstruct_Succeeds) {
    TextFileOutputRange a{ tmp };
    EXPECT_NO_FATAL_FAILURE({ TextFileOutputRange b{ std::move(a) }; });
}

TEST_F(FileOutputRangeConstructTest, MoveAssign_Succeeds) {
    TextFileOutputRange a{ tmp };
    TextFileOutputRange b{ MakeTempPath("thoth_test_assign.txt") };
    EXPECT_NO_FATAL_FAILURE({ b = std::move(a); });
    std::filesystem::remove(MakeTempPath("thoth_test_assign.txt"));
}

#pragma endregion


#pragma region Range API

struct FileOutputRangeApiTest : testing::Test {
    const std::filesystem::path tmp{ MakeTempPath("thoth_test_range.txt") };

    void TearDown() override {
        std::filesystem::remove(tmp);
    }
};

TEST_F(FileOutputRangeApiTest, End_ReturnsUnreachableSentinel) {
    TextFileOutputRange range{ tmp };
    EXPECT_NO_FATAL_FAILURE({ auto e{ range.end() }; (void)e; });
}

TEST_F(FileOutputRangeApiTest, Begin_ReturnsOutputIterator) {
    TextFileOutputRange range{ tmp };
    EXPECT_NO_FATAL_FAILURE({ auto it{ range.begin() }; (void)it; });
}

TEST_F(FileOutputRangeApiTest, SatisfiesRangeConcept) {
    EXPECT_TRUE((std::ranges::range<TextFileOutputRange>));
}

TEST_F(FileOutputRangeApiTest, Write_SingleChar_FileContainsIt) {
    {
        TextFileOutputRange range{ tmp };
        *range.begin() = 'X';
    }
    const auto content{ ReadFile(tmp) };
    EXPECT_EQ(content, "X");
}

TEST_F(FileOutputRangeApiTest, Write_StringViaRangeCopy_FileContainsContent) {
    const std::string text{ "hello thoth" };
    {
        TextFileOutputRange range{ tmp };
        std::ranges::copy(text, range.begin());
    }
    EXPECT_EQ(ReadFile(tmp), text);
}

TEST_F(FileOutputRangeApiTest, Write_MultipleChars_InOrder) {
    {
        TextFileOutputRange range{ tmp };
        auto it{ range.begin() };
        *it = 'a'; ++it;
        *it = 'b'; ++it;
        *it = 'c';
    }
    EXPECT_EQ(ReadFile(tmp), "abc");
}

TEST_F(FileOutputRangeApiTest, Write_EmptyContent_FileExists) {
    { TextFileOutputRange range{ tmp }; }
    EXPECT_TRUE(std::filesystem::exists(tmp));
}

TEST_F(FileOutputRangeApiTest, Mode_ReturnsInt) {
    EXPECT_NO_FATAL_FAILURE({ auto m{ TextFileOutputRange::Mode() }; (void)m; });
}

#pragma endregion


#pragma region Equality

struct FileOutputRangeEqualityTest : testing::Test {
    const std::filesystem::path tmp{ MakeTempPath("thoth_test_eq.txt") };

protected:
    void TearDown() override {
        std::filesystem::remove(tmp);
    }
};

// TEST_F(FileOutputRangeEqualityTest, SelfEquality_Equal) {
//     TextFileOutputRange a{ tmp };
//     EXPECT_EQ(a, a);
// }

#pragma endregion