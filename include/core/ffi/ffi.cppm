export module core_ffi;
// bridge partition moved to core_plugin:loader

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::ffi {

// ─── StringLiteral ────────────────────────────────────────────────────────────
// Structural NTTP wrapper so string literals can be template parameters — same trick as
// serde::StringLiteral (include/serde/core.cppm), duplicated here (not imported from serde)
// so core_ffi stays standalone, no cross-dependency on the serde module.
template <std::size_t N>
struct StringLiteral {
    /**
     * @brief Copies a string literal's characters (including the trailing NUL) into `value`
     * at compile time — this is the whole trick that lets a literal ride along as a template
     * parameter.
     * @param source the string literal being wrapped, e.g. `"has_task_type"`.
     */
    consteval StringLiteral(const char (&source)[N]) noexcept { std::copy_n(source, N, m_value); }
    char m_value[N]{};

    /// @brief Gets a view over the literal, trailing NUL excluded.
    /// @return a `string_view` of length `N - 1` over `m_value`.
    [[nodiscard]] constexpr std::string_view string_view() const noexcept { return {m_value, N - 1}; }
};

// ─── MethodDesc / Exported<T> ─────────────────────────────────────────────────
// Hand-written FFI method export list, mirroring serde::FieldDesc/Serializable<T>'s
// reflection-free convention: no C++26 static reflection available on this toolchain
// (confirmed — __cpp_reflection is undefined, no experimental flag enables it either), so a
// type opts into FFI export by providing an explicit core::ffi::Exported<T> specialization
// listing exactly which member functions get bridged to Python/Lua, instead of the host
// auto-enumerating every member.
//
// TODO(reflection): this whole file (StringLiteral, MethodDesc, Exported<T>, and every
// hand-written specialization of it, e.g. sdk/worker/task.cppm's Exported<TaskRunner>)
// becomes unnecessary once the toolchain has real P2996 static reflection — GCC 16.1
// actually implements it today via `-std=c++26 -freflection` (confirmed by direct testing:
// `std::meta::nonstatic_data_members_of`/`^^Foo` both compile and run), just not on the
// clang toolchain this project currently builds with — clang's own P2996 support only
// exists in a separate experimental fork (bloomberg/clang-p2996), not upstream. Once
// available on this project's actual compiler, register_class<T>() should go back to
// reflecting over T's members directly (see the #ifdef __cpp_reflection block removed from
// include/core/manager/ffi.cppm's register_class — that's the shape to restore), and this
// marker convention can be deleted.

/// @brief One exported member function: a compile-time name paired with the actual
/// pointer-to-member-function `FfiRuntime::register_class` binds against.
/// @tparam Name the name this method is exposed under (used to build both the internal
/// registered key and the Python/Lua-facing name).
/// @tparam MemFn the pointer-to-member-function being exported.
template <StringLiteral Name, auto MemFn>
struct MethodDesc {
    static constexpr auto name = Name;
    static constexpr auto member = MemFn;
};

/// @brief Marker template — every FFI-exported type provides an explicit specialization, never
/// the primary template. Mirrors `serde::Serializable<T>`'s convention exactly.
/// @details A specialization must provide:
/// - `static constexpr auto methods()` returning `std::tuple<MethodDesc<...>, ...>` of every
///   member function to expose.
/// - `static T &instance()` returning the single instance `register_class<T>()` binds every
///   exported method against (typically a function-local static — this is what makes
///   build-time auto-discovery of exported types possible without needing per-type
///   construction logic wired in by hand at the call site).
template <typename T>
struct Exported;

/// @brief Checks whether `T` has opted into FFI export via an `Exported<T>` specialization.
template <typename T>
concept IsExported = requires {
    { Exported<T>::methods() };
    { Exported<T>::instance() } -> std::same_as<T &>;
};

} // namespace core::ffi

#ifdef CONGELADO_TEST
namespace core::ffi::tests {

class FfiTestTarget {
  public:
    [[nodiscard]] int add_one(int value) const noexcept { return value + 1; }
};

class FfiUnexportedTarget {};

} // namespace core::ffi::tests

namespace core::ffi {

// Explicit specialization must live in a namespace enclosing core::ffi::Exported's own
// namespace — hence this sits directly in core::ffi rather than core::ffi::tests.
template <>
struct Exported<tests::FfiTestTarget> {
    static constexpr auto methods() {
        return std::tuple{MethodDesc<"add_one", &tests::FfiTestTarget::add_one>{}};
    }
    static tests::FfiTestTarget &instance() {
        static tests::FfiTestTarget target;
        return target;
    }
};

} // namespace core::ffi

namespace core::ffi::tests {
using namespace boost::ut;

suite<"ffi::StringLiteral"> string_literal_suite = [] {
    "wraps a literal and exposes it as a string_view"_test = [] {
        constexpr StringLiteral name = "has_task_type";
        expect(name.string_view() == "has_task_type");
        expect(name.string_view().size() == 13);
    };
};

suite<"ffi::MethodDesc"> method_desc_suite = [] {
    "carries a name and a bound member-function pointer"_test = [] {
        using Desc = MethodDesc<"add_one", &FfiTestTarget::add_one>;
        expect(Desc::name.string_view() == "add_one");

        FfiTestTarget target;
        expect((target.*Desc::member)(4) == 5);
    };
};

suite<"ffi::IsExported"> is_exported_suite = [] {
    "a type with an Exported<T> specialization satisfies the concept"_test = [] {
        expect(IsExported<FfiTestTarget>);
    };
    "a type without a specialization does not satisfy the concept"_test = [] {
        expect(not IsExported<FfiUnexportedTarget>);
    };
};

} // namespace core::ffi::tests
#endif
