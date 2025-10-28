#pragma once

namespace Thoth::Utils {
    // thanks https://en.cppreference.com/w/cpp/utility/variant/visit2.html
    template<class... Ts>
    struct Overloaded : Ts... { using Ts::operator()...; };
}