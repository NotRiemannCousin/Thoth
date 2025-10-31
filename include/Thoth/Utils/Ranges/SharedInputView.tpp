#pragma once

namespace Thoth::Utils {
    template<std::ranges::input_range Range>
    typename SharedInputView<Range>::Iterator::value_type SharedInputView<Range>::Iterator::operator*() const {
        return *view->_current;
    }

    template<std::ranges::input_range Range>
    typename SharedInputView<Range>::Iterator& SharedInputView<Range>::Iterator::operator++() {
        ++view->_current;
        return *this;
    }

    template<std::ranges::input_range Range>
    typename SharedInputView<Range>::Iterator& SharedInputView<Range>::Iterator::operator++(int) {
        return ++(*this);
    }

    template<std::ranges::input_range Range>
    bool SharedInputView<Range>::Iterator::operator==(std::default_sentinel_t) const {
        return view->_current == view->_end;
    }


    template<std::ranges::input_range Range>
    SharedInputView<Range>::SharedInputView(Range&& range)
        : _current(std::ranges::begin(range))
        , _end(std::ranges::end(range)) {
    }


    template<std::ranges::input_range Range>
    typename SharedInputView<Range>::Iterator SharedInputView<Range>::begin() {
        return Iterator{this};
    }

    template<std::ranges::input_range Range>
    std::default_sentinel_t SharedInputView<Range>::end() {
        return {};
    }
}