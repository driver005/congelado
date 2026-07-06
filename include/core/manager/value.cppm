module;
#include <congelado/abi.h>
export module core_plugin:value;
import std;

export namespace core::plugin {

struct None {};
struct Int {
    std::int64_t v;
};
struct Float {
    double v;
};
struct Bool {
    bool v;
};
struct Str {
    std::string v;
};

class Map;
class Array;

using Value =
    std::variant<None, Int, Float, Bool, Str, std::shared_ptr<Map>, std::shared_ptr<Array>, void *>;

class Map {
  public:
    Map() = default;
    Map(const Map &) = delete;
    Map &operator=(const Map &) = delete;
    Map(Map &&) = default;
    Map &operator=(Map &&) = default;

    void set(std::string key, Value val) {
        m_entries.insert_or_assign(std::move(key), std::move(val));
    }
    [[nodiscard]] const Value *get(std::string_view key) const noexcept {
        auto it = m_entries.find(std::string{key});
        return it != m_entries.end() ? &it->second : nullptr;
    }
    [[nodiscard]] std::size_t get_size() const noexcept { return m_entries.size(); }
    template <typename Self>
    [[nodiscard]] auto &get_entries(this Self &&self) noexcept {
        return self.m_entries;
    }

  private:
    std::unordered_map<std::string, Value> m_entries;
};

class Array {
  public:
    Array() = default;
    Array(const Array &) = delete;
    Array &operator=(const Array &) = delete;
    Array(Array &&) = default;
    Array &operator=(Array &&) = default;
    explicit Array(std::size_t cap) { m_items.reserve(cap); }
    void push(Value v) { m_items.push_back(std::move(v)); }
    [[nodiscard]] const Value *get(std::size_t i) const noexcept {
        return i < m_items.size() ? &m_items[i] : nullptr;
    }
    [[nodiscard]] std::size_t get_size() const noexcept { return m_items.size(); }
    template <typename Self>
    [[nodiscard]] auto &get_items(this Self &&self) noexcept {
        return self.m_items;
    }

  private:
    std::vector<Value> m_items;
};

template <typename T>
struct ValueTraits;

template <>
struct ValueTraits<bool> {
    static Value to_value(bool v) { return Bool{v}; }
    static bool from_value(const Value &val) {
        return std::visit(
            []<typename U>(const U &x) -> bool {
                if constexpr (std::same_as<U, Bool>)
                    return x.v;
                if constexpr (std::same_as<U, Int>)
                    return x.v != 0;
                throw std::runtime_error{"expected bool"};
            },
            val);
    }
};
template <std::integral T>
    requires(!std::same_as<T, bool>)
struct ValueTraits<T> {
    static Value to_value(T v) { return Int{static_cast<std::int64_t>(v)}; }
    T from_value(const Value &val) {
        return static_cast<T>(std::visit(
            []<typename U>(const U &x) -> std::int64_t {
                if constexpr (std::same_as<U, Int>)
                    return x.v;
                if constexpr (std::same_as<U, Bool>)
                    return x.v ? 1 : 0;
                throw std::runtime_error{"expected int"};
            },
            val));
    }
};
template <std::floating_point T>
struct ValueTraits<T> {
    static Value to_value(T v) { return Float{static_cast<double>(v)}; }
    T from_value(const Value &val) {
        return static_cast<T>(std::visit(
            []<typename U>(const U &x) -> double {
                if constexpr (std::same_as<U, Float>)
                    return x.v;
                if constexpr (std::same_as<U, Int>)
                    return static_cast<double>(x.v);
                throw std::runtime_error{"expected float"};
            },
            val));
    }
};
template <>
struct ValueTraits<std::string> {
    static Value to_value(std::string v) { return Str{std::move(v)}; }
    static std::string from_value(const Value &val) {
        return std::visit(
            []<typename T>(const T &x) -> std::string {
                if constexpr (std::same_as<T, Str>)
                    return x.v;
                throw std::runtime_error{"expected string"};
            },
            val);
    }
};
template <>
struct ValueTraits<std::string_view> {
    static Value to_value(std::string_view v) { return Str{std::string{v}}; }
    static std::string_view from_value(const Value &val) {
        return std::visit(
            []<typename T>(const T &x) -> std::string_view {
                if constexpr (std::same_as<T, Str>)
                    return x.v;
                throw std::runtime_error{"expected string_view"};
            },
            val);
    }
};
template <>
struct ValueTraits<Value> {
    static Value to_value(Value v) { return v; }
    static const Value &from_value(const Value &v) { return v; }
};
template <typename T>
    requires(std::is_aggregate_v<T> && !std::is_array_v<T>)
struct ValueTraits<T> {
    static Value to_value(const T &v) {
        auto m = std::make_shared<Map>();
#ifdef __cpp_reflection
        constexpr auto ctx = std::meta::access_context::current();
        template for (constexpr auto f : std::meta::nonstatic_data_members_of(^^T, ctx)) {
            constexpr std::string_view name = std::meta::identifier_of(f);
            if (name.starts_with('_'))
                continue;
            using FT = std::remove_cvref_t<decltype(v.[:f:])>;
            m->set(std::string{name}, ValueTraits<FT>::to_value(v.[:f:]));
        }
#endif
        return m;
    }
    static T from_value(const Value &val) {
        const auto *mp = std::get_if<std::shared_ptr<Map>>(&val);
        if (!mp || !*mp)
            throw std::runtime_error{std::string{"expected Map for "} + typeid(T).name()};
        const Map &m = **mp;
        T result{};
#ifdef __cpp_reflection
        constexpr auto ctx = std::meta::access_context::current();
        template for (constexpr auto f : std::meta::nonstatic_data_members_of(^^T, ctx)) {
            constexpr std::string_view name = std::meta::identifier_of(f);
            if (name.starts_with('_'))
                continue;
            using FT = std::remove_cvref_t<decltype(result.[:f:])>;
            const Value *slot = m.get(name);
            if (!slot)
                throw std::runtime_error{std::format("map missing '{}'", name)};
            result.[:f:] = ValueTraits<FT>::from_value(*slot);
        }
#endif
        return result;
    }
};

// ── AnyConverter — converts between Value and CongeladoAny ─────────────────

class AnyConverter {
  public:
    AnyConverter() = delete;

    // Forward declaration — HandleTable defined in ffi.cppm
    class HandleTable;

    [[nodiscard]] static Value from_any(const CongeladoAny &a) {
        switch (a.type_index) {
        case CG_NONE:
            return None{};
        case CG_INT:
        case CG_BOOL:
            return Int{a.v_int64};
        case CG_FLOAT:
            return Float{a.v_float64};
        case CG_STR:
            return Str{a.v_cstr ? std::string{a.v_cstr} : ""};
        case CG_MAP: {
            auto *raw = static_cast<Map *>(a.v_ptr);
            return std::shared_ptr<Map>(raw, [](Map *) {});
        }
        case CG_ARRAY: {
            auto *raw = static_cast<Array *>(a.v_ptr);
            return std::shared_ptr<Array>(raw, [](Array *) {});
        }
        default:
            return a.v_ptr;
        }
    }

    [[nodiscard]] static CongeladoAny to_any(Value &v) {
        CongeladoAny a{};
        std::visit(
            [&]<typename T>(T &x) {
                if constexpr (std::same_as<T, None>) {
                    a.type_index = CG_NONE;
                } else if constexpr (std::same_as<T, Bool>) {
                    a.type_index = CG_BOOL;
                    a.v_int64 = x.v ? 1 : 0;
                } else if constexpr (std::same_as<T, Int>) {
                    a.type_index = CG_INT;
                    a.v_int64 = x.v;
                } else if constexpr (std::same_as<T, Float>) {
                    a.type_index = CG_FLOAT;
                    a.v_float64 = x.v;
                } else if constexpr (std::same_as<T, Str>) {
                    a.type_index = CG_STR;
                    a.v_cstr = x.v.c_str();
                } else if constexpr (std::same_as<T, std::shared_ptr<Map>>) {
                    a.type_index = CG_MAP;
                    a.v_ptr = x.get();
                } else if constexpr (std::same_as<T, std::shared_ptr<Array>>) {
                    a.type_index = CG_ARRAY;
                    a.v_ptr = x.get();
                } else if constexpr (std::same_as<T, void *>) {
                    a.type_index = CG_PTR;
                    a.v_ptr = x;
                }
            },
            v);
        return a;
    }

    [[nodiscard]] static CongeladoAny to_any_borrow(const Value &v) {
        CongeladoAny a{};
        std::visit(
            [&]<typename T>(const T &x) {
                if constexpr (std::same_as<T, None>) {
                    a.type_index = CG_NONE;
                } else if constexpr (std::same_as<T, Bool>) {
                    a.type_index = CG_BOOL;
                    a.v_int64 = x.v ? 1 : 0;
                } else if constexpr (std::same_as<T, Int>) {
                    a.type_index = CG_INT;
                    a.v_int64 = x.v;
                } else if constexpr (std::same_as<T, Float>) {
                    a.type_index = CG_FLOAT;
                    a.v_float64 = x.v;
                } else if constexpr (std::same_as<T, Str>) {
                    a.type_index = CG_STR;
                    a.v_cstr = x.v.c_str();
                } else if constexpr (std::same_as<T, std::shared_ptr<Map>>) {
                    a.type_index = CG_MAP;
                    a.v_ptr = x.get();
                } else if constexpr (std::same_as<T, std::shared_ptr<Array>>) {
                    a.type_index = CG_ARRAY;
                    a.v_ptr = x.get();
                } else if constexpr (std::same_as<T, void *>) {
                    a.type_index = CG_PTR;
                    a.v_ptr = x;
                }
            },
            v);
        return a;
    }
};

// ── HandleTable — type-erased map/array handle storage ──────────────────────

class HandleTable {
  public:
    HandleTable() = delete;

    [[nodiscard]] static int64_t insert(std::any obj) {
        auto id = s_next_id++;
        s_handles[id] = std::move(obj);
        return id;
    }

    [[nodiscard]] static std::any &get(int64_t id) { return s_handles.at(id); }

    static void remove(int64_t id) { s_handles.erase(id); }

    template <typename T>
    [[nodiscard]] static T get_as(int64_t id) {
        return std::any_cast<T>(s_handles.at(id));
    }

    // ── Map handle API ──────────────────────────────────────────────────

    [[nodiscard]] static CongeladoAny map_create() noexcept {
        auto id = insert(std::make_shared<Map>());
        return CongeladoAny{.type_index = CG_MAP_HANDLE, .v_int64 = id};
    }

    static void map_set(const CongeladoAny *h, const CongeladoAny *k,
                        const CongeladoAny *v) noexcept {
        auto map = get_as<std::shared_ptr<Map>>(h->v_int64);
        map->set(k->v_cstr ? std::string{k->v_cstr} : "", AnyConverter::from_any(*v));
    }

    [[nodiscard]] static CongeladoAny map_get(const CongeladoAny *h,
                                              const CongeladoAny *k) noexcept {
        auto map = get_as<std::shared_ptr<Map>>(h->v_int64);
        const auto *val = map->get(k->v_cstr ? std::string_view{k->v_cstr} : "");
        if (!val)
            return CongeladoAny{};
        return AnyConverter::to_any_borrow(*val);
    }

    [[nodiscard]] static CongeladoAny get_map_size(const CongeladoAny *h) noexcept {
        auto map = get_as<std::shared_ptr<Map>>(h->v_int64);
        return CongeladoAny{.type_index = CG_INT, .v_int64 = static_cast<int64_t>(map->get_size())};
    }

    [[nodiscard]] static CongeladoAny get_map_keys(const CongeladoAny *h) noexcept {
        auto map = get_as<std::shared_ptr<Map>>(h->v_int64);
        auto arr = std::make_shared<Array>();
        for (const auto &[key, _] : map->get_entries())
            (*arr).push(Str{key});
        auto id = insert(std::move(arr));
        return CongeladoAny{.type_index = CG_ARRAY_HANDLE, .v_int64 = id};
    }

    static void handle_free(const CongeladoAny *h) noexcept { remove(h->v_int64); }

    // ── Array handle API ────────────────────────────────────────────────

    [[nodiscard]] static CongeladoAny array_create(const CongeladoAny *cap) noexcept {
        std::size_t c =
            (cap && cap->type_index == CG_INT) ? static_cast<std::size_t>(cap->v_int64) : 0;
        auto id = insert(std::make_shared<Array>(c));
        return CongeladoAny{.type_index = CG_ARRAY_HANDLE, .v_int64 = id};
    }

    static void array_push(const CongeladoAny *h, const CongeladoAny *v) noexcept {
        auto arr = get_as<std::shared_ptr<Array>>(h->v_int64);
        arr->push(AnyConverter::from_any(*v));
    }

    [[nodiscard]] static CongeladoAny get_array_size(const CongeladoAny *h) noexcept {
        auto arr = get_as<std::shared_ptr<Array>>(h->v_int64);
        return CongeladoAny{.type_index = CG_INT, .v_int64 = static_cast<int64_t>(arr->get_size())};
    }

    [[nodiscard]] static CongeladoAny array_get(const CongeladoAny *h,
                                                const CongeladoAny *i) noexcept {
        auto arr = get_as<std::shared_ptr<Array>>(h->v_int64);
        std::size_t idx = (i && i->type_index == CG_INT) ? static_cast<std::size_t>(i->v_int64) : 0;
        const auto *val = arr->get(idx);
        if (!val)
            return CongeladoAny{};
        return AnyConverter::to_any_borrow(*val);
    }

  private:
    static inline std::unordered_map<int64_t, std::any> s_handles;
    static inline int64_t s_next_id = 1;
};

} // namespace core::plugin
