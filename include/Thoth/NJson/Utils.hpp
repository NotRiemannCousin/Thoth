#pragma once

namespace Thoth::NJson {
    // I really don't know why I made this two

    //! @brief Constructs a Json::Array from a variadic list of values.
    //! @details
    //! Creates a `std::vector<Json>` (i.e., `Thoth::NJson::Array`) containing one
    //! element for each argument, in order. Each argument must be constructible to
    //! `Thoth::NJson::Json`. This is a convenient way to build JSON arrays directly
    //! from C++ values.
    //!
    //! **Input requirements:**
    //! - Every `T` in `ts...` must satisfy `std::constructible_from<Json, T>`. This includes
    //!   arithmetic types, bool, string-like types, existing Json objects/arrays, and null.
    //! - The number of arguments may be zero (produces an empty array).
    //!
    //! **Return value:**
    //! - Returns an Array by value. The array may be moved into a Json value or used
    //!   directly in further JSON construction.
    //!
    //! @tparam T Types convertible to Json.
    //! @param ts Values to insert into the array.
    //! @return `Thoth::NJson::Array` containing all values.
    template<class ...T>
        requires (std::constructible_from<Json::Value, T>, ...)
    Array MakeArray(T&&... ts) {
        Array arr{};
        arr.reserve(sizeof...(T));

        (arr.emplace_back(std::forward<T>(ts)), ...);

        return arr;
    }

    //! @brief Constructs a Json::Object from a variadic list of key-value pairs.
    //! @details
    //! Creates a `Thoth::NJson::JsonObject` with one entry for each provided pair.
    //! Each `ts` is expected to be a pair-like type (e.g., `std::pair`, `std::tuple`
    //! of size 2) where `.first` is a key (convertible to `std::string`) and `.second`
    //! is a value (constructible to `Json`).
    //!
    //! **Input requirements:**
    //! - For each `P` in `Ps...`: `P` must provide `.first` and `.second` accessible
    //!   via member access or `get`. The `.first` should be string-constructible;
    //!   the `.second` must satisfy `std::constructible_from<Json>`.
    //! - Duplicate keys are allowed; later entries overwrite earlier ones (as per
    //!   JsonObject::Set semantics).
    //!
    //! **Return value:**
    //! - Returns a JsonObject by value. You can assign it directly to a Json value.
    //!
    //! @tparam P Pair-like types representing key-value entries.
    //! @param ts Key-value pairs to insert into the object.
    //! @return `Thoth::NJson::JsonObject` containing all entries.
    template<class... P>
        requires (std::constructible_from<Json, typename P::second_type> && ...)
    JsonObject MakeObject(P&&... ts) {
        return JsonObject{
            JsonObject::JsonPair(
                std::forward<P>(ts).first,
                Json{ std::forward<P>(ts).second }
            )...
        };
    }
}