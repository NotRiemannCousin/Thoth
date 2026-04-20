#pragma once
#include <string_view>
#include <optional>
#include <variant>
#include <iomanip>
#include <format>
#include <chrono>
#include <string>

#include <Thoth/String/Utils.hpp>
#include <Thoth/Utils/Monostate.hpp>
#include <Thoth/Utils/Scanner.hpp>
namespace Thoth::Http::NHeaders {

    enum class HeaderErrorEnum {
        NotFound,
        InvalidFormat,
        EmptyValue
    };
    struct InvalidHeaderFormat {};
}
