#include <gtest/gtest.h>
#include <Thoth/NJson/Json.hpp>
#include <Thoth/NJson/JsonObject.hpp>
 

using namespace Thoth::NJson;
 
#pragma region Helpers

static Json ParseOk(std::string_view input) {
    auto result{ Json::Parse(input) };
    EXPECT_TRUE(result) << "Parse failed for: " << input;
    return result.value_or(Json{});
}

#pragma endregion

 
#pragma region Construction & Type Detection

struct JsonTypeTest : testing::Test {
    Json nullJ{};
    Json boolJ{ true };
    Json intJ{ 42 };
    Json doubleJ{ 3.14 };
    Json strJ{ std::string("hello") };
    Json arrJ{ Array{} };
    Json objJ{ JsonObject{} };
};
 
TEST_F(JsonTypeTest, DefaultConstruct_IsNull) {
    EXPECT_TRUE(nullJ.IsOf<Null>());
}
 
TEST_F(JsonTypeTest, BoolConstruct_IsBool) {
    EXPECT_TRUE(boolJ.IsOf<Bool>());
}
 
TEST_F(JsonTypeTest, IntConstruct_IsNumber) {
    EXPECT_TRUE(intJ.IsOf<Number>());
}
 
TEST_F(JsonTypeTest, DoubleConstruct_IsNumber) {
    EXPECT_TRUE(doubleJ.IsOf<Number>());
}
 
TEST_F(JsonTypeTest, StringConstruct_IsString) {
    EXPECT_TRUE(strJ.IsOf<String>());
}
 
TEST_F(JsonTypeTest, ArrayConstruct_IsArray) {
    EXPECT_TRUE(arrJ.IsOf<Array>());
}
 
TEST_F(JsonTypeTest, ObjectConstruct_IsObject) {
    EXPECT_TRUE(objJ.IsOf<Object>());
}
 
TEST_F(JsonTypeTest, IsOfType_StaticVariant) {
    EXPECT_TRUE(Json::IsOfType<Bool>(boolJ));
    EXPECT_FALSE(Json::IsOfType<Number>(boolJ));
}
 
TEST_F(JsonTypeTest, As_Bool_ReturnsValue) {
    EXPECT_EQ(boolJ.As<Bool>(), true);
}
 
TEST_F(JsonTypeTest, As_Number_ReturnsValue) {
    EXPECT_DOUBLE_EQ(static_cast<double>(intJ.As<Number>()), 42.0);
}
 
TEST_F(JsonTypeTest, CopyAssign_Number) {
    Json j{};
    j = 99;
    EXPECT_TRUE(j.IsOf<Number>());
    EXPECT_DOUBLE_EQ(static_cast<double>(j.As<Number>()), 99.0);
}
 
TEST_F(JsonTypeTest, CopyAssign_Bool) {
    Json j{};
    j = false;
    EXPECT_TRUE(j.IsOf<Bool>());
    EXPECT_EQ(j.As<Bool>(), false);
}
 
TEST_F(JsonTypeTest, Equality_SameNull) {
    EXPECT_EQ(Json{}, Json{});
}
 
TEST_F(JsonTypeTest, Equality_SameBool) {
    EXPECT_EQ(Json{ true }, Json{ true });
}
 
TEST_F(JsonTypeTest, Inequality_DifferentTypes) {
    EXPECT_NE(Json{ true }, Json{ 1 });
}

#pragma endregion

 
#pragma region Parsing

struct JsonParseTest : testing::Test {};
 
TEST_F(JsonParseTest, Parse_Null_Succeeds) {
    const auto r{ Json::Parse("null") };
    ASSERT_TRUE(r);
    EXPECT_TRUE(r->IsOf<Null>());
}
 
TEST_F(JsonParseTest, Parse_BoolTrue_Succeeds) {
    const auto r{ Json::Parse("true") };
    ASSERT_TRUE(r);
    EXPECT_EQ(r->As<Bool>(), true);
}
 
TEST_F(JsonParseTest, Parse_BoolFalse_Succeeds) {
    const auto r{ Json::Parse("false") };
    ASSERT_TRUE(r);
    EXPECT_EQ(r->As<Bool>(), false);
}
 
TEST_F(JsonParseTest, Parse_Integer_Succeeds) {
    const auto r{ Json::Parse("42") };
    ASSERT_TRUE(r);
    EXPECT_DOUBLE_EQ(static_cast<double>(r->As<Number>()), 42.0);
}
 
TEST_F(JsonParseTest, Parse_NegativeNumber_Succeeds) {
    const auto r{ Json::Parse("-7") };
    ASSERT_TRUE(r);
    EXPECT_DOUBLE_EQ(static_cast<double>(r->As<Number>()), -7.0);
}
 
TEST_F(JsonParseTest, Parse_Float_Succeeds) {
    const auto r{ Json::Parse("3.14") };
    ASSERT_TRUE(r);
    EXPECT_NEAR(static_cast<double>(r->As<Number>()), 3.14, 1e-9);
}
 
TEST_F(JsonParseTest, Parse_SimpleString_Succeeds) {
    const auto r{ Json::Parse(R"("hello")") };
    ASSERT_TRUE(r);
    EXPECT_TRUE(r->IsOf<String>());
    EXPECT_EQ(r->As<String>().AsCopy(), "hello");
}
 
TEST_F(JsonParseTest, Parse_EmptyString_Succeeds) {
    const auto r{ Json::Parse(R"("")") };
    ASSERT_TRUE(r);
    EXPECT_EQ(r->As<String>().AsCopy(), "");
}
 
TEST_F(JsonParseTest, Parse_StringWithEscapes_Succeeds) {
    const auto r{ Json::Parse(R"("line1\nline2")") };
    ASSERT_TRUE(r);
    EXPECT_EQ(r->As<String>().AsCopy(), "line1\nline2");
}
 
TEST_F(JsonParseTest, Parse_EmptyArray_Succeeds) {
    const auto r{ Json::Parse("[]") };
    ASSERT_TRUE(r);
    EXPECT_TRUE(r->IsOf<Array>());
    EXPECT_TRUE(r->As<Array>().empty());
}
 
TEST_F(JsonParseTest, Parse_Array_CorrectSize) {
    const auto r{ Json::Parse("[1, 2, 3]") };
    ASSERT_TRUE(r);
    ASSERT_TRUE(r->IsOf<Array>());
    EXPECT_EQ(r->As<Array>().size(), 3u);
}
 
TEST_F(JsonParseTest, Parse_Array_CorrectElements) {
    const auto r{ Json::Parse("[true, null, 5]") };
    ASSERT_TRUE(r);
    const auto& arr{ r->As<Array>() };
    EXPECT_TRUE(arr[0].IsOf<Bool>());
    EXPECT_TRUE(arr[1].IsOf<Null>());
    EXPECT_TRUE(arr[2].IsOf<Number>());
}
 
TEST_F(JsonParseTest, Parse_EmptyObject_Succeeds) {
    const auto r{ Json::Parse("{}") };
    ASSERT_TRUE(r);
    EXPECT_TRUE(r->IsOf<Object>());
    EXPECT_TRUE(r->As<Object>()->Empty());
}
 
TEST_F(JsonParseTest, Parse_SimpleObject_CorrectKeys) {
    const auto r{ Json::Parse(R"({"name":"Alice","age":30})") };
    ASSERT_TRUE(r);
    ASSERT_TRUE(r->IsOf<Object>());
    const auto& obj{ *r->As<Object>() };
    EXPECT_TRUE(obj.Exists("name"));
    EXPECT_TRUE(obj.Exists("age"));
}
 
TEST_F(JsonParseTest, Parse_NestedObject_Succeeds) {
    const auto r{ Json::Parse(R"({"user":{"id":1,"active":true}})") };
    ASSERT_TRUE(r);
}
 
TEST_F(JsonParseTest, Parse_NestedArray_Succeeds) {
    const auto r{ Json::Parse("[[1,2],[3,4]]") };
    ASSERT_TRUE(r);
    const auto& outer{ r->As<Array>() };
    ASSERT_EQ(outer.size(), 2u);
    EXPECT_TRUE(outer[0].IsOf<Array>());
}
 
TEST_F(JsonParseTest, Parse_Invalid_ReturnsError) {
    EXPECT_FALSE(Json::Parse("not json"));
}
 
TEST_F(JsonParseTest, Parse_Empty_ReturnsError) {
    EXPECT_FALSE(Json::Parse(""));
}
 
TEST_F(JsonParseTest, Parse_UnclosedObject_ReturnsError) {
    EXPECT_FALSE(Json::Parse(R"({"key": 1)"));
}
 
TEST_F(JsonParseTest, Parse_UnclosedArray_ReturnsError) {
    EXPECT_FALSE(Json::Parse("[1, 2"));
}
 
TEST_F(JsonParseTest, Parse_TrailingGarbage_CheckFinalTrue_ReturnsError) {
    EXPECT_FALSE(Json::ParseText("42 garbage", true, true));
}
 
TEST_F(JsonParseTest, Parse_TrailingGarbage_CheckFinalFalse_Succeeds) {
    EXPECT_TRUE(Json::ParseText("42 garbage", true, false));
}

#pragma endregion

 
#pragma region Ensure / EnsureOrError

struct JsonEnsureTest : testing::Test {
    Json numJ{ 10 };
    Json boolJ{ false };
};
 
TEST_F(JsonEnsureTest, Ensure_CorrectType_HasValue) {
    EXPECT_TRUE(numJ.Ensure<Number>());
}
 
TEST_F(JsonEnsureTest, Ensure_WrongType_ReturnsNullopt) {
    EXPECT_FALSE(numJ.Ensure<Bool>());
}
 
TEST_F(JsonEnsureTest, EnsureOrError_CorrectType_HasValue) {
    EXPECT_TRUE(numJ.EnsureOrError<Number>());
}
 
TEST_F(JsonEnsureTest, EnsureOrError_WrongType_HasError) {
    const auto result{ numJ.EnsureOrError<Bool>() };
    EXPECT_FALSE(result);
    EXPECT_TRUE(std::holds_alternative<Thoth::Http::JsonWrongTypeError>(result.error()));
}
 
TEST_F(JsonEnsureTest, EnsureRef_CorrectType_HasValue) {
    EXPECT_TRUE(boolJ.EnsureRef<Bool>());
}

#pragma endregion

 
#pragma region Get / GetOrError on Json (Object & Array navigation)

struct JsonGetTest : testing::Test {
    Json obj{ ParseOk(R"({"name":"Bob","score":99,"active":true})") };
    Json arr{ ParseOk("[10, 20, 30]") };
};
 
TEST_F(JsonGetTest, Get_ByStringKey_ReturnsPtr) {
    auto result{ obj.Get(Key{ std::string("name") }) };
    ASSERT_TRUE(result);
    EXPECT_TRUE((*result)->IsOf<String>());
}
 
TEST_F(JsonGetTest, Get_NonExistentKey_ReturnsNullopt) {
    auto result{ obj.Get(Key{ std::string("missing") }) };
    EXPECT_FALSE(result);
}
 
TEST_F(JsonGetTest, Get_ByIndex_ReturnsElement) {
    auto result{ arr.Get(Key{ 0 }) };
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(static_cast<double>((*result)->As<Number>()), 10.0);
}
 
TEST_F(JsonGetTest, Get_IndexOutOfRange_ReturnsNullopt) {
    auto result{ arr.Get(Key{ 99 }) };
    EXPECT_FALSE(result);
}
 
TEST_F(JsonGetTest, Get_NegativeIndex_ReturnsFromEnd) {
    // -1 = last element
    auto result{ arr.Get(Key{ -1 }) };
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(static_cast<double>((*result)->As<Number>()), 30.0);
}
 
TEST_F(JsonGetTest, GetOrError_ExistingKey_HasValue) {
    auto result{ obj.GetOrError(Key{ std::string("score") }) };
    EXPECT_TRUE(result);
}
 
TEST_F(JsonGetTest, GetOrError_MissingKey_HasError) {
    auto result{ obj.GetOrError(Key{ std::string("ghost") }) };
    EXPECT_FALSE(result);
    EXPECT_TRUE(std::holds_alternative<Thoth::Http::JsonGetError>(result.error()));
}
 
TEST_F(JsonGetTest, GetCopy_ReturnsIndependentCopy) {
    auto copy{ obj.GetCopy(Key{ std::string("score") }) };
    ASSERT_TRUE(copy);
    *copy = 0;
    // Original must be untouched
    auto original{ obj.GetOrError(Key{ std::string("score") }) };
    ASSERT_TRUE(original);
    EXPECT_DOUBLE_EQ(static_cast<double>((*original)->As<Number>()), 99.0);
}
 
TEST_F(JsonGetTest, GetCopyOrNull_Missing_ReturnsNull) {
    auto result{ obj.GetCopyOrNull(Key{ std::string("ghost") }) };
    EXPECT_TRUE(result.IsOf<Null>());
}

#pragma endregion

 
#pragma region Find (deep traversal)

struct JsonFindTest : testing::Test {
    // {"data": {"users": [{"name": "Alice"}, {"name": "Bob"}]}}
    Json root{ ParseOk(R"({"data":{"users":[{"name":"Alice"},{"name":"Bob"}]}})") };
};
 
TEST_F(JsonFindTest, Find_TwoLevels_Succeeds) {
    const std::array keys{ Key{ std::string("data") }, Key{ std::string("users") } };
    auto result{ root.Find(keys) };
    ASSERT_TRUE(result);
    EXPECT_TRUE((*result)->IsOf<Array>());
}
 
TEST_F(JsonFindTest, Find_ThreeLevels_Succeeds) {
    const std::array keys{
        Key{ std::string("data") },
        Key{ std::string("users") },
        Key{ 0 },
        Key{ std::string("name") }
    };
    auto result{ root.Find(keys) };
    ASSERT_TRUE(result);
    EXPECT_EQ((*result)->As<String>().AsCopy(), "Alice");
}
 
TEST_F(JsonFindTest, Find_InvalidPath_ReturnsNullopt) {
    const std::array keys{ Key{ std::string("data") }, Key{ std::string("ghost") } };
    auto result{ root.Find(keys) };
    EXPECT_FALSE(result);
}
 
TEST_F(JsonFindTest, FindOrError_ValidPath_HasValue) {
    const std::array keys{
        Key{ std::string("data") },
        Key{ std::string("users") },
        Key{ 1 },
        Key{ std::string("name") }
    };
    auto result{ root.FindOrError(keys) };
    ASSERT_TRUE(result);
    EXPECT_EQ((*result)->As<String>().AsCopy(), "Bob");
}
 
TEST_F(JsonFindTest, FindOrError_InvalidPath_HasError) {
    const std::array keys{ Key{ std::string("data") }, Key{ std::string("nope") } };
    auto result{ root.FindOrError(keys) };
    EXPECT_FALSE(result);
}
 
TEST_F(JsonFindTest, FindCopyOrNull_InvalidPath_ReturnsNull) {
    const std::array keys{ Key{ std::string("no") }, Key{ std::string("way") } };
    auto result{ root.FindCopyOrNull(keys) };
    EXPECT_TRUE(result.IsOf<Null>());
}

#pragma endregion

 
#pragma region Search

struct JsonSearchTest : testing::Test {
    // [1, "find-me", true, null]
    Json arr{ ParseOk(R"([1, "find-me", true, null])") };
    // {"a": 1, "b": "target", "c": false}
    Json obj{ ParseOk(R"({"a":1,"b":"target","c":false})") };
};
 
TEST_F(JsonSearchTest, Search_Array_MatchFound_ReturnsPtrToElement) {
    auto pred = [](const Json& j) { return j.IsOf<String>(); };
    auto result{ arr.Search(pred) };
    ASSERT_TRUE(result);
    EXPECT_EQ((*result)->As<String>().AsCopy(), "find-me");
}
 
TEST_F(JsonSearchTest, Search_Array_NoMatch_ReturnsNullopt) {
    auto pred = [](const Json& j) { return j.IsOf<Object>(); };
    auto result{ arr.Search(pred) };
    EXPECT_FALSE(result);
}
 
TEST_F(JsonSearchTest, Search_Object_MatchFound) {
    auto pred = [](const Json& j) { return j.IsOf<String>(); };
    auto result{ obj.Search(pred) };
    ASSERT_TRUE(result);
    EXPECT_EQ((*result)->As<String>().AsCopy(), "target");
}
 
TEST_F(JsonSearchTest, SearchCopy_ReturnsIndependentCopy) {
    auto pred = [](const Json& j) { return j.IsOf<Bool>(); };
    auto copy{ arr.SearchCopy(pred) };
    ASSERT_TRUE(copy);
    EXPECT_EQ(copy->As<Bool>(), true);
}
 
TEST_F(JsonSearchTest, SearchOrError_NoMatch_HasError) {
    auto pred = [](const Json& j) { return j.IsOf<Array>(); };
    auto result{ arr.SearchOrError(pred) };
    EXPECT_FALSE(result);
    EXPECT_TRUE(std::holds_alternative<Thoth::Http::JsonSearchError>(result.error()));
}
 
TEST_F(JsonSearchTest, SearchCopyOrNull_NoMatch_ReturnsNull) {
    auto pred = [](const Json& j) { return j.IsOf<Object>(); };
    auto result{ arr.SearchCopyOrNull(pred) };
    EXPECT_TRUE(result.IsOf<Null>());
}

#pragma endregion

 
#pragma region Visit

struct JsonVisitTest : testing::Test {};
 
TEST_F(JsonVisitTest, Visit_Number_CallsNumberLambda) {
    Json j{ 7 };
    std::string visited;
    j.Visit(Thoth::Utils::Overloaded{
        [&](Number)  { visited = "number"; },
        [&](Bool)    { visited = "bool"; },
        [&](String&) { visited = "string"; },
        [&](Object&) { visited = "object"; },
        [&](Array&)  { visited = "array"; },
        [&](Null)    { visited = "null"; }
    });
    EXPECT_EQ(visited, "number");
}
 
TEST_F(JsonVisitTest, Visit_Const_InvokesCorrectHandler) {
    const Json j{ true };
    bool wasBool{ false };
    j.Visit(Thoth::Utils::Overloaded{
        [&](Number)        {},
        [&](Bool b)        { wasBool = b; },
        [&](const String&) {},
        [&](const Object&) {},
        [&](const Array&)  {},
        [&](Null)          {}
    });
    EXPECT_TRUE(wasBool);
}

#pragma endregion

 
#pragma region Formatting

struct JsonFormatTest : testing::Test {};
 
TEST_F(JsonFormatTest, Format_Null) {
    EXPECT_EQ(std::format("{}", Json{}), "null");
}
 
TEST_F(JsonFormatTest, Format_BoolTrue) {
    EXPECT_EQ(std::format("{}", Json{ true }), "true");
}
 
TEST_F(JsonFormatTest, Format_BoolFalse) {
    EXPECT_EQ(std::format("{}", Json{ false }), "false");
}
 
TEST_F(JsonFormatTest, Format_Number_Integer) {
    EXPECT_EQ(std::format("{}", Json{ 42 }), "42");
}
 
TEST_F(JsonFormatTest, Format_String) {
    EXPECT_EQ(std::format("{}", Json{ std::string("hi") }), R"("hi")");
}
 
TEST_F(JsonFormatTest, Format_StringWithSpecialChars_Escapes) {
    Json j{ std::string("a\"b\\c\nd") };
    const auto formatted{ std::format("{}", j) };
    EXPECT_EQ(formatted, R"("a\"b\\c\nd")");
}
 
TEST_F(JsonFormatTest, Format_EmptyArray) {
    EXPECT_EQ(std::format("{}", ParseOk("[]")), "[]");
}
 
TEST_F(JsonFormatTest, Format_SimpleArray) {
    const auto j{ ParseOk("[1,2,3]") };
    const auto s{ std::format("{}", j) };
    EXPECT_EQ(s, "[1,2,3]");
}
 
TEST_F(JsonFormatTest, Format_EmptyObject) {
    EXPECT_EQ(std::format("{}", ParseOk("{}")), "{}");
}
 
TEST_F(JsonFormatTest, Format_RoundTrip_Object) {
    const std::string input{ R"({"a":1,"b":true})" };
    const auto j{ ParseOk(input) };
    const auto formatted{ std::format("{}", j) };
    const auto reparsed{ Json::Parse(formatted) };
    ASSERT_TRUE(reparsed);
    EXPECT_EQ(*reparsed, j);
}

#pragma endregion

 
#pragma region JsonObject

struct JsonObjectTest : testing::Test {
    JsonObject obj{
        { "key1", Json{ 1 } },
        { "key2", Json{ std::string("value") } },
        { "key3", Json{ true } }
    };
};
 
TEST_F(JsonObjectTest, Exists_Key_True) {
    EXPECT_TRUE(obj.Exists("key1"));
}
 
TEST_F(JsonObjectTest, Exists_Key_False) {
    EXPECT_FALSE(obj.Exists("missing"));
}
 
TEST_F(JsonObjectTest, Size_ReturnsCount) {
    EXPECT_EQ(obj.Size(), 3u);
}
 
TEST_F(JsonObjectTest, Empty_FalseForNonEmpty) {
    EXPECT_FALSE(obj.Empty());
}
 
TEST_F(JsonObjectTest, Empty_TrueAfterClear) {
    JsonObject tmp{ obj };
    tmp.Clear();
    EXPECT_TRUE(tmp.Empty());
}
 
TEST_F(JsonObjectTest, Get_ExistingKey_HasValue) {
    auto result{ obj.Get("key1") };
    ASSERT_TRUE(result);
    EXPECT_TRUE((*result)->IsOf<Number>());
}
 
TEST_F(JsonObjectTest, Get_MissingKey_ReturnsNullopt) {
    EXPECT_FALSE(obj.Get("nope"));
}
 
TEST_F(JsonObjectTest, Set_UpdatesExistingKey) {
    JsonObject tmp{ obj };
    Json newVal{ 999 };
    tmp.Set("key1", newVal);
    const auto result{ tmp.Get("key1") };
    ASSERT_TRUE(result);
    EXPECT_DOUBLE_EQ(static_cast<double>((*result)->As<Number>()), 999.0);
}
 
TEST_F(JsonObjectTest, Set_AddsNewKey) {
    JsonObject tmp{ obj };
    Json newVal{ std::string("fresh") };
    tmp.Set("newkey", newVal);
    EXPECT_TRUE(tmp.Exists("newkey"));
    EXPECT_EQ(tmp.Size(), 4u);
}
 
TEST_F(JsonObjectTest, Remove_ExistingKey_ReturnsTrue) {
    JsonObject tmp{ obj };
    EXPECT_TRUE(tmp.Remove("key1"));
    EXPECT_FALSE(tmp.Exists("key1"));
    EXPECT_EQ(tmp.Size(), 2u);
}
 
TEST_F(JsonObjectTest, Remove_MissingKey_ReturnsFalse) {
    JsonObject tmp{ obj };
    EXPECT_FALSE(tmp.Remove("ghost"));
}
 
TEST_F(JsonObjectTest, SetIfNull_NewKey_ReturnsTrueAndSets) {
    JsonObject tmp{ obj };
    Json v{ 77 };
    EXPECT_TRUE(tmp.SetIfNull("newkey", v));
    EXPECT_TRUE(tmp.Exists("newkey"));
}
 
TEST_F(JsonObjectTest, SetIfNull_ExistingKey_ReturnsFalseAndDoesNotChange) {
    JsonObject tmp{ obj };
    const auto original{ tmp.Get("key1") };
    Json v{ 9999 };
    EXPECT_FALSE(tmp.SetIfNull("key1", v));
    const auto result{ tmp.Get("key1") };
    ASSERT_TRUE(result);
    EXPECT_EQ(result, original);
}
 
TEST_F(JsonObjectTest, OperatorBracket_CreatesKeyIfMissing) {
    JsonObject tmp{ obj };
    tmp["new"] = Json{ 5 };
    EXPECT_TRUE(tmp.Exists("new"));
}
 
TEST_F(JsonObjectTest, Equality_SameContents_Equal) {
    JsonObject a{
        { "x", Json{ 1 } },
        { "y", Json{ 2 } }
    };
    JsonObject b{
        { "x", Json{ 1 } },
        { "y", Json{ 2 } }
    };
    EXPECT_EQ(a, b);
}
 
TEST_F(JsonObjectTest, Equality_DifferentContents_NotEqual) {
    JsonObject a{ { "x", Json{ 1 } } };
    JsonObject b{ { "x", Json{ 2 } } };
    EXPECT_NE(a, b);
}
 
TEST_F(JsonObjectTest, IterateKeys_AllKeysPresent) {
    std::vector<std::string> keys;
    for (const auto& [k, v] : obj)
        keys.push_back(k);
    EXPECT_EQ(keys.size(), 3u);
}
 
TEST_F(JsonObjectTest, GetCopy_ReturnsValue) {
    const auto copy{ obj.GetCopy("key2") };
    ASSERT_TRUE(copy);
    EXPECT_EQ(copy->As<String>().AsCopy(), "value");
}
 
TEST_F(JsonObjectTest, GetCopyOrNull_Missing_IsNull) {
    const auto result{ obj.GetCopyOrNull("nope") };
    EXPECT_TRUE(result.IsOf<Null>());
}

#pragma endregion