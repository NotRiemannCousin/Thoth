#pragma once
#include <ranges>
#include <memory>
#include <iterator>

namespace Thoth::Utils {
    template<std::ranges::input_range Range>
    struct SharedInputView {
        struct Iterator {
            using difference_type = std::ptrdiff_t;
            using value_type = std::ranges::range_value_t<Range>;

            SharedInputView* view = nullptr;

            [[nodiscard]] value_type operator*() const;
            Iterator& operator++();
            Iterator& operator++(int);
            [[nodiscard]] bool operator==(std::default_sentinel_t) const;
        };

        explicit SharedInputView(Range&& range);

        Iterator begin();
        static std::default_sentinel_t end();

    private:
        std::ranges::iterator_t<Range> _current;
        std::ranges::sentinel_t<Range> _end;
    };
}

#include <Thoth/Utils/Ranges/SharedInputView.tpp>

namespace Thoth::Utils {
    static_assert(std::ranges::range<SharedInputView<std::ranges::iota_view<int, int>>>);
}