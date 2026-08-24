#pragma once
#include <Thoth/NJson/Definitions.hpp>

namespace Thoth::NJson {
    //! @see Thoth::ThothError "ThothError" is the centralized error type of this library.
    struct JsonParseError {
        size_t idx;
        char c;
    };

    //! @see Thoth::ThothError "ThothError" is the centralized error type of this library.
    struct JsonGetError {
        Key key;
    };

    //! @see Thoth::ThothError "ThothError" is the centralized error type of this library.
    struct JsonFindError {
        Key key;
        std::vector<Key> currentPath;
    };

    //! @see Thoth::ThothError "ThothError" is the centralized error type of this library.
    struct JsonSearchError { };

    //! @see Thoth::ThothError "ThothError" is the centralized error type of this library.
    struct JsonWrongTypeError {
        template<class T>
        static constexpr size_t IndexOf{};

        size_t idxExpected{};
        size_t idxGot{};
    };

    // TODO: FUTURE: high obscure, change to some fancy way in the future, but that will do
    // std::variant<Null, String, Number, Bool, Object, Array>;
    template<> constexpr size_t JsonWrongTypeError::IndexOf<Null  >{ 0 };
    template<> constexpr size_t JsonWrongTypeError::IndexOf<String>{ 1 };
    template<> constexpr size_t JsonWrongTypeError::IndexOf<Number>{ 2 };
    template<> constexpr size_t JsonWrongTypeError::IndexOf<Bool  >{ 3 };
    template<> constexpr size_t JsonWrongTypeError::IndexOf<Object>{ 4 };
    template<> constexpr size_t JsonWrongTypeError::IndexOf<Array >{ 5 };
}

#include <Thoth/NJson/ErrorDefinitions.tpp>