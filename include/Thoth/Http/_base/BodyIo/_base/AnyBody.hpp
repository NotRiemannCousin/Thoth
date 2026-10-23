#pragma once
#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>

#include <exec/task.hpp>

#include <Thoth/Http/_base/BodyIo/_base.hpp>

namespace Thoth::Http {

    //! @brief Which sides of an exchange a body can feed, the minimum @ref BodyIo surface an erased body promises.
    //!
    //! `In`  = the transport pushes wire bytes in (`Write` + `FinishWrite`, the Server request side).
    //! `Out` = the transport pulls slices out (`Size` + `Read`, the Server response side).
    //! `Both` = the body models both directions at once (symmetric bodies, a proxy pipe, an echo buffer).
    enum class BodyDirection : std::uint8_t { In, Out, Both };

    namespace details_ {
        template<class> struct FunctionRef;

        //! @brief Low budget std::function_ref.
        //! @note Automatically checks for data and function pointer validity.
        template <class Ret, class... Args>
        struct FunctionRef<Ret(void*, Args...)> {
            Ret (*fn)(void*, Args...){};

            constexpr FunctionRef& operator=(Ret (*f)(void*, Args...)) noexcept;

            template <class Fallback>
            constexpr Ret operator()(void* data, Fallback&& fallback, Args... args) const;

            constexpr void operator()(void* data, Args... args) const noexcept;

            constexpr explicit operator bool() const noexcept;
        };

        //! @brief The shape of the erased I/O surface, one static entry per @ref BodyIo member.
        struct AnyBodyVTable {
            using SizeFunc   = std::optional<std::uint64_t>(void*);
            using ReadFunc   = exec::task<std::optional<ByteView>>(void*);
            using WriteFunc  = exec::task<WriteResult>(void*, std::span<std::byte>);
            using FinishFunc = exec::task<void>(void*);

            FunctionRef<SizeFunc>   size{};
            FunctionRef<ReadFunc>   read{};
            FunctionRef<WriteFunc>  write{};
            FunctionRef<FinishFunc> finishWrite{};

            AnyBodyVTable() = default;

            template<BodyConcept BodyT>
            explicit AnyBodyVTable(std::in_place_type_t<BodyT>);
        };
    }

    template<BodyDirection Dir>
    struct AnyBody;

    //! @brief The @ref BodyIo bound to an erased body, the specialization that keeps the CPO link alive.
    template<BodyDirection Dir>
    struct BodyIo<AnyBody<Dir>> {
        [[nodiscard]] std::optional<std::uint64_t> Size(const AnyBody<Dir>& body) const requires (Dir != BodyDirection::In);
        exec::task<std::optional<ByteView>>        Read(const AnyBody<Dir>& body) requires (Dir != BodyDirection::In);

        exec::task<WriteResult> Write(AnyBody<Dir>& body, std::span<std::byte> window) requires (Dir != BodyDirection::Out);
        exec::task<void>        FinishWrite(AnyBody<Dir>& body)                        requires (Dir != BodyDirection::Out);
    };

    //! @brief A type-erased body (still fixed with its @ref BodyDirection, @ref BodyDirection::Both can convert to any
    //! AnyBody).
    template<BodyDirection Dir>
    struct AnyBody {
        static constexpr BodyDirection direction{ Dir };

        AnyBody() noexcept = default;

        template<BodyDirection OtherDir>
        friend struct AnyBody;

        template<BodyConcept BodyT>
            requires (Dir == BodyDirection::Out  && OutgoingBodyConcept<BodyT>)
                  || (Dir == BodyDirection::In   && IncomingBodyConcept<BodyT>)
                  || (Dir == BodyDirection::Both && SymetricBodyConcept<BodyT>)
        AnyBody(BodyT&& body)
            requires (!std::same_as<std::remove_cvref_t<BodyT>, AnyBody>);

        AnyBody(AnyBody&&)            noexcept = default;
        AnyBody& operator=(AnyBody&&) noexcept = default;


        AnyBody(AnyBody<BodyDirection::Both>&& other) noexcept requires (Dir != BodyDirection::Both);

        AnyBody(const AnyBody&)            = delete;
        AnyBody& operator=(const AnyBody&) = delete;

        [[nodiscard]] std::optional<std::uint64_t> Size() const requires (Dir != BodyDirection::In);
        exec::task<std::optional<ByteView>>        Read() const requires (Dir != BodyDirection::In);

        exec::task<WriteResult> Write(std::span<std::byte> window) requires (Dir != BodyDirection::Out);
        exec::task<void>        FinishWrite()                      requires (Dir != BodyDirection::Out);

        [[nodiscard]] BodyIoFor<AnyBody>&       Io()       noexcept;
        [[nodiscard]] const BodyIoFor<AnyBody>& Io() const noexcept;

        [[nodiscard]] explicit operator bool() const noexcept;

    private:
        using Deleter = details_::FunctionRef<void(void*)>;

        std::unique_ptr<void, Deleter> m_data{ nullptr };
        details_::AnyBodyVTable m_vtable{};
        [[no_unique_address]] BodyIoFor<AnyBody> m_io{};
    };

    using AnyBodyIn   = AnyBody<BodyDirection::In>;
    using AnyBodyOut  = AnyBody<BodyDirection::Out>;
    using AnyBodyBoth = AnyBody<BodyDirection::Both>;
}

#include <Thoth/Http/_base/BodyIo/_base/AnyBody.tpp>

namespace Thoth::Http {
    static_assert(Thoth::Http::OutgoingBodyConcept<AnyBodyOut>);
    static_assert(Thoth::Http::IncomingBodyConcept<AnyBodyIn>);
    static_assert(Thoth::Http::SymetricBodyConcept<AnyBodyBoth>);
}