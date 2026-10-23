#pragma once
#include <Thoth/Utils/Await.hpp>

namespace Thoth::Http {
    namespace details_ {
        template<class Ret, class ... Args>
        constexpr FunctionRef<Ret(void*, Args...)>& FunctionRef<Ret(void*, Args...)>::operator=(
        Ret(* f)(void*, Args...)) noexcept {
            fn = f;
            return *this;
        }

        template<class Ret, class... Args>
        template<class Fallback>
        constexpr Ret FunctionRef<Ret(void*, Args...)>::operator()(void* data, Fallback&& fallback, Args... args) const {
            if (data && fn) return fn(data, std::forward<Args>(args)...);
            return std::forward<Fallback>(fallback)();
        }

        template<class Ret, class... Args>
        constexpr void FunctionRef<Ret(void*, Args...)>::operator()(void* data, Args... args) const noexcept {
            if (data && fn) fn(data, std::forward<Args>(args)...);
        }

        template<class Ret, class ... Args>
        constexpr FunctionRef<Ret(void*, Args...)>::operator bool() const noexcept {
            return fn != nullptr;
        }


        //! @brief Group up the body with it's BodyIo to simplify AnyBody's implementation.
        template<BodyConcept BodyT>
        struct AnyBodySlot {
            BodyT            body;
            BodyIoFor<BodyT> io{};
        };
        template<BodyConcept BodyT>
        using AnyBodySlotFor = AnyBodySlot<std::remove_cvref_t<BodyT>>;


        //! @brief Normalized `BodyIo<BodyT>` via erasing.
        template<class Body>
        struct AnyBodyThunks {
            using BodyT = std::remove_cvref_t<Body>;

            static std::optional<std::uint64_t> Size(void* slot) {
                const auto& body{ *static_cast<const AnyBodySlot<BodyT>*>(slot) };
                return body.io.Size(body.body);
            }
            static exec::task<std::optional<ByteView>> Read(void* slot) {
                auto& body{ *static_cast<AnyBodySlot<BodyT>*>(slot) };
                co_return co_await Utils::Await(body.io.Read(body.body));
            }
            static exec::task<WriteResult> Write(void* slot, std::span<std::byte> window) {
                auto& body{ *static_cast<AnyBodySlot<BodyT>*>(slot) };
                co_return co_await Utils::Await(body.io.Write(body.body, window));
            }
            static exec::task<void> FinishWrite(void* slot) {
                auto& body{ *static_cast<AnyBodySlot<BodyT>*>(slot) };
                co_await Utils::Await(body.io.FinishWrite(body.body));
            }
            static void Destroy(void* slot) noexcept {
                delete static_cast<AnyBodySlot<BodyT>*>(slot);
            }
        };

        template<BodyConcept BodyT>
        AnyBodyVTable::AnyBodyVTable(std::in_place_type_t<BodyT>) {
            if constexpr (OutgoingBodyConcept<BodyT>) {
                size = &AnyBodyThunks<BodyT>::Size;
                read = &AnyBodyThunks<BodyT>::Read;
            }

            if constexpr (IncomingBodyConcept<BodyT>) {
                write       = &AnyBodyThunks<BodyT>::Write;
                finishWrite = &AnyBodyThunks<BodyT>::FinishWrite;
            }
        }
    }



    template<BodyDirection Dir>
    std::optional<std::uint64_t> BodyIo<AnyBody<Dir>>::Size(const AnyBody<Dir>& body) const requires (Dir != BodyDirection::In) {
        return body.Size();
    }

    template<BodyDirection Dir>
    exec::task<std::optional<ByteView>> BodyIo<AnyBody<Dir>>::Read(const AnyBody<Dir>& body) requires (Dir != BodyDirection::In) {
        co_return co_await body.Read();
    }

    template<BodyDirection Dir>
    exec::task<WriteResult> BodyIo<AnyBody<Dir>>::Write(AnyBody<Dir>& body, std::span<std::byte> window) requires (Dir != BodyDirection::Out) {
        co_return co_await body.Write(window);
    }

    template<BodyDirection Dir>
    exec::task<void> BodyIo<AnyBody<Dir>>::FinishWrite(AnyBody<Dir>& body)
        requires (Dir != BodyDirection::Out) {
        co_await body.FinishWrite();
    }

    template<BodyDirection Dir>
    template<BodyConcept BodyT>
        requires (Dir == BodyDirection::Out  && OutgoingBodyConcept<BodyT>)
              || (Dir == BodyDirection::In   && IncomingBodyConcept<BodyT>)
              || (Dir == BodyDirection::Both && SymetricBodyConcept<BodyT>)
    AnyBody<Dir>::AnyBody(BodyT&& body)
        requires (!std::same_as<std::remove_cvref_t<BodyT>, AnyBody>)
            : m_data{ new details_::AnyBodySlotFor<BodyT>{ std::forward<BodyT>(body) },
                      Deleter{ &details_::AnyBodyThunks<BodyT>::Destroy } },
              m_vtable{ details_::AnyBodyVTable{ std::in_place_type<BodyT> } } {}


    template<BodyDirection Dir>
    AnyBody<Dir>::AnyBody(AnyBody<BodyDirection::Both>&& other) noexcept requires (Dir != BodyDirection::Both)
        : m_data{ std::move(other.m_data) }, m_vtable{ other.m_vtable } {}

    template<BodyDirection Dir>
    std::optional<std::uint64_t> AnyBody<Dir>::Size() const requires (Dir != BodyDirection::In) {
        return m_vtable.size(m_data.get(), [] -> std::uint64_t { return 0; });
    }

    template<BodyDirection Dir>
    exec::task<std::optional<ByteView>> AnyBody<Dir>::Read() const requires (Dir != BodyDirection::In) {
        return m_vtable.read(m_data.get(), []() -> exec::task<std::optional<ByteView>> { co_return std::nullopt; });
    }

    template<BodyDirection Dir>
    exec::task<WriteResult> AnyBody<Dir>::Write(std::span<std::byte> window) requires (Dir != BodyDirection::Out) {
        return m_vtable.write(m_data.get(), [s = window.size()]() -> exec::task<WriteResult> {
            co_return WriteResult{ s, {} };
        }, window);
    }

    template<BodyDirection Dir>
    exec::task<void> AnyBody<Dir>::FinishWrite() requires (Dir != BodyDirection::Out) {
        return m_vtable.finishWrite(m_data.get(), []() -> exec::task<void> { co_return; });
    }

    template<BodyDirection Dir>
    BodyIoFor<AnyBody<Dir>>& AnyBody<Dir>::Io() noexcept            { return m_io; }

    template<BodyDirection Dir>
    const BodyIoFor<AnyBody<Dir>>& AnyBody<Dir>::Io() const noexcept { return m_io; }

    template<BodyDirection Dir>
    AnyBody<Dir>::operator bool() const noexcept { return m_data != nullptr; }
}
