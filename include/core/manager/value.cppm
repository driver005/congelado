// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
module;
#include <congelado/abi.h>
export module core_plugin:value;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::plugin {

struct None
{};

struct Int
{
    std::int64_t m_value;
};

struct Float
{
    double m_value;
};

struct Bool
{
    bool m_value;
};

struct Str
{
    std::string m_value;
};

class Map;
class Array;

using Value =
    std::variant<None, Int, Float, Bool, Str, std::shared_ptr<Map>, std::shared_ptr<Array>, void*>;

class Map
{
public:
    /// @brief Default-constructs an empty Map.
    Map() = default;

    /// @brief Destroys the Map, releasing every entry's Value.
    ~Map() = default;

    /// @brief Deleted — Maps hold potentially-large entry sets, no implicit copying that
    /// motion.
    Map(const Map&) = delete;
    /// @brief Deleted — same reason as the copy ctor.
    Map& operator=(const Map&) = delete;

    /// @brief Move-constructs, stealing the other Map's entries.
    Map(Map&& other) noexcept :
        m_entries(std::move(other.m_entries))
    {
    }

    /// @brief Move-assigns, stealing the other Map's entries.
    Map& operator=(Map&& other) noexcept
    {
        if (this != &other) {
            m_entries = std::move(other.m_entries);
        }
        return *this;
    }

    /**
     * @brief Sets a key to a value, inserting or overwriting as needed.
     * @param key the entry key.
     * @param val the value to store.
     */
    void set(std::string key, Value val)
    {
        m_entries.insert_or_assign(std::move(key), std::move(val));
    }

    /**
     * @brief Looks up a value by key.
     * @param key the entry key to look up.
     * @return a copy of the stored Value, or nullopt if the key isn't present.
     */
    [[nodiscard]] std::optional<Value> get(std::string_view key) const noexcept
    {
        auto it = m_entries.find(std::string{key});
        return it != m_entries.end() ? std::make_optional(it->second) : std::nullopt;
    }

    /// @brief Gets how many entries are stored.
    /// @return the entry count.
    [[nodiscard]] std::size_t get_size() const noexcept
    {
        return m_entries.size();
    }

    /// @brief Gets the full backing entry map.
    /// @return a reference to the underlying key/Value map.
    [[nodiscard]] const std::unordered_map<std::string, Value>& get_entries() noexcept
    {
        return m_entries;
    }

private:
    std::unordered_map<std::string, Value> m_entries;
};

class Array
{
public:
    /// @brief Default-constructs an empty Array with no reserved capacity.
    Array() = default;

    /// @brief Constructs an empty Array, pre-reserving storage for `capacity` items.
    /// @param capacity number of items to reserve space for up front.
    explicit Array(std::size_t capacity)
    {
        m_items.reserve(capacity);
    }

    /// @brief Destroys the Array, releasing every stored Value.
    ~Array() = default;

    /// @brief Deleted — Arrays hold potentially-large item sets, no implicit copying that
    /// motion.
    Array(const Array&) = delete;
    /// @brief Deleted — same reason as the copy ctor.
    Array& operator=(const Array&) = delete;

    /// @brief Move-constructs, stealing the other Array's items.
    Array(Array&& other) noexcept :
        m_items(std::move(other.m_items))
    {
    }

    /// @brief Move-assigns, stealing the other Array's items.
    Array& operator=(Array&& other) noexcept
    {
        if (this != &other) {
            m_items = std::move(other.m_items);
        }
        return *this;
    }

    /// @brief Appends a value to the end of the array.
    /// @param value the value to append.
    void push(Value value)
    {
        m_items.push_back(std::move(value));
    }

    /**
     * @brief Looks up a value by index.
     * @param idx the zero-based index to look up.
     * @return a copy of the stored Value, or nullopt if `idx` is out of range.
     */
    [[nodiscard]] std::optional<Value> get(std::size_t idx) const noexcept
    {
        return idx < m_items.size()
                   ? std::make_optional(m_items[idx])
                   : std::nullopt; // FIXME(clang-tidy): unchecked operator[], consider .at()
    }

    /// @brief Gets how many items are stored.
    /// @return the item count.
    [[nodiscard]] std::size_t get_size() const noexcept
    {
        return m_items.size();
    }

    /// @brief Gets a mutable view over every stored item.
    /// @return a span over the underlying item storage.
    [[nodiscard]] std::span<Value> get_items() noexcept
    {
        return m_items;
    }

private:
    std::vector<Value> m_items;
};

template<typename T>
struct ValueTraits;

template<>
struct ValueTraits<bool>
{
    /// @brief Wraps a `bool` as a Value.
    /// @param val the bool to wrap.
    /// @return the equivalent Value (a `Bool` alternative).
    static Value to_value(bool val)
    {
        return Bool{val};
    }

    /**
     * @brief Unwraps a Value back into a `bool`.
     * @param val the Value to unwrap; accepts `Bool` directly or `Int` (nonzero → true).
     * @return the extracted bool.
     * @throws std::runtime_error if `val` holds neither `Bool` nor `Int`.
     */
    static bool from_value(const Value& val)
    {
        // Accept either a direct Bool or an Int coerced to bool (nonzero == true) —
        // anything else in the variant is a real type mismatch, so it throws.
        return std::visit(
            []<typename U>(const U& value) -> bool {
                if constexpr (std::same_as<U, Bool>) {
                    return value.m_value;
                }
                if constexpr (std::same_as<U, Int>) {
                    return value.m_value != 0;
                }
                throw std::runtime_error{"expected bool"};
            },
            val
        );
    }
};

template<std::integral T>
    requires(!std::same_as<T, bool>)
struct ValueTraits<T>
{
    /// @brief Wraps an integral value as a Value.
    /// @param val the integral to wrap, narrowed/widened to `std::int64_t`.
    /// @return the equivalent Value (an `Int` alternative).
    static Value to_value(T val)
    {
        return Int{static_cast<std::int64_t>(val)};
    }

    /**
     * @brief Unwraps a Value back into `T`.
     * @warning This is straight cooked as written: it's missing `static` (unlike every other
     * ValueTraits::from_value in this file) so it can't be called the way the rest of the
     * codebase calls ValueTraits — as `ValueTraits<T>::from_value(...)` with no instance.
     * Worse, the body reads `value.value`/`value.m_value`... actually reads `.value`, but
     * `Int`/`Bool` only declare `m_value` — that member doesn't exist. Neither bug surfaces
     * until this template actually gets instantiated for some integral `T`, since templates
     * aren't checked until instantiation. No cap, this won't compile the day someone calls it.
     * @param val the Value to unwrap; intended to accept `Int` directly or `Bool` (as 0/1).
     * @return the extracted value as `T`.
     * @throws std::runtime_error if `val` holds neither `Int` nor `Bool`.
     */
    T from_value(const Value& val)
    {
        // Intended flow: accept a direct Int as-is, or coerce a Bool to 0/1, then
        // narrow the visited int64 result down to T via the outer static_cast.
        return static_cast<T>(std::visit(
            []<typename U>(const U& value) -> std::int64_t {
                if constexpr (std::same_as<U, Int>) {
                    return value.value;
                }
                if constexpr (std::same_as<U, Bool>) {
                    return value.value ? 1 : 0;
                }
                throw std::runtime_error{"expected int"};
            },
            val
        ));
    }
};

template<std::floating_point T>
struct ValueTraits<T>
{
    /// @brief Wraps a floating-point value as a Value.
    /// @param val the value to wrap, converted to `double`.
    /// @return the equivalent Value (a `Float` alternative).
    static Value to_value(T val)
    {
        return Float{static_cast<double>(val)};
    }

    /**
     * @brief Unwraps a Value back into `T`.
     * @warning Same footgun as the integral ValueTraits::from_value right above: missing
     * `static`, and the body reaches for `value.value` when `Float`/`Int` only expose
     * `m_value`. Both issues only bite at instantiation time (never checked for an
     * uninstantiated template), so this is a landmine, not a currently-firing bug.
     * @param val the Value to unwrap; intended to accept `Float` directly or `Int` (widened).
     * @return the extracted value as `T`.
     * @throws std::runtime_error if `val` holds neither `Float` nor `Int`.
     */
    T from_value(const Value& val)
    {
        // Intended flow: accept a direct Float as-is, or widen an Int to double,
        // then narrow the visited double result down to T via the outer static_cast.
        return static_cast<T>(std::visit(
            []<typename U>(const U& value) -> double {
                if constexpr (std::same_as<U, Float>) {
                    return value.value;
                }
                if constexpr (std::same_as<U, Int>) {
                    return static_cast<double>(value.value);
                }
                throw std::runtime_error{"expected float"};
            },
            val
        ));
    }
};

template<>
struct ValueTraits<std::string>
{
    /// @brief Wraps a `std::string` as a Value.
    /// @param val the string to wrap (moved in).
    /// @return the equivalent Value (a `Str` alternative).
    static Value to_value(std::string val)
    {
        return Str{std::move(val)};
    }

    /**
     * @brief Unwraps a Value back into a `std::string`.
     * @param val the Value to unwrap; must hold `Str`.
     * @return a copy of the extracted string.
     * @throws std::runtime_error if `val` doesn't hold `Str`.
     */
    static std::string from_value(const Value& val)
    {
        // Only Str unwraps cleanly here — everything else in the variant throws,
        // no cap, no implicit stringification of Int/Float/Bool.
        return std::visit(
            []<typename T>(const T& value) -> std::string {
                if constexpr (std::same_as<T, Str>) {
                    return value.m_value;
                }
                throw std::runtime_error{"expected string"};
            },
            val
        );
    }
};

template<>
struct ValueTraits<std::string_view>
{
    /// @brief Wraps a `std::string_view` as a Value, copying it into an owned `Str`.
    /// @param val the string_view to wrap; the underlying characters are copied.
    /// @return the equivalent Value (a `Str` alternative).
    static Value to_value(std::string_view val)
    {
        return Str{std::string{val}};
    }

    /**
     * @brief Unwraps a Value into a `std::string_view` borrowing the Value's own storage.
     * @warning The returned view is only valid as long as `val` (and its underlying `Str`)
     * stays alive — let `val` go out of scope or get reassigned first and this view dangles.
     * @param val the Value to unwrap; must hold `Str`.
     * @return a view over the extracted string's characters.
     * @throws std::runtime_error if `val` doesn't hold `Str`.
     */
    static std::string_view from_value(const Value& val)
    {
        // Same shape as the std::string overload, just returning a borrowed view
        // into the Str's own storage instead of copying it out.
        return std::visit(
            []<typename T>(const T& value) -> std::string_view {
                if constexpr (std::same_as<T, Str>) {
                    return value.m_value;
                }
                throw std::runtime_error{"expected string_view"};
            },
            val
        );
    }
};

// template <>
// struct ValueTraits<Value> {
//     static Value to_value(Value val) { return val; }
//     static const Value &from_value(const Value &val) { return val; }
// };

template<typename T>
    requires(std::is_aggregate_v<T> && !std::is_array_v<T>)
struct ValueTraits<T>
{
    /**
     * @brief Wraps an aggregate struct as a Value, reflecting over its non-underscore-prefixed
     * fields and recursively converting each one.
     * @note Only compiled with `__cpp_reflection` — without it this just returns an empty Map,
     * silently dropping every field. That's not a thrown error, so don't sleep on checking for
     * reflection support if you're relying on this for a real struct.
     * @param val the aggregate to convert.
     * @return a Map Value with one entry per reflected field.
     */
    static Value to_value(const T& val)
    {
        auto map = std::make_shared<Map>();
#ifdef __cpp_reflection
        // Walk every non-static data member reflected off T, skipping anything
        // underscore-prefixed (treated as "private"/non-serialized by convention),
        // and recursively convert the rest into map entries keyed by field name.
        constexpr auto ctx = std::meta::access_context::current();
        template for (constexpr auto field: std::meta::nonstatic_data_members_of(^^T, ctx))
        {
            constexpr std::string_view name = std::meta::identifier_of(field);
            if (name.starts_with('_')) {
                continue;
            }
            using FT = std::remove_cvref_t<decltype(val.[:field:])>;
            map->set(std::string{name}, ValueTraits<FT>::to_value(val.[:field:]));
        }
#endif
        return map;
    }

    /**
     * @brief Unwraps a Map Value back into an aggregate `T`, reflecting over its fields.
     * @note Only compiled with `__cpp_reflection` — without it this just returns a
     * default-constructed `T{}` with every field left at its default, no error raised.
     * @param val the Value to unwrap; must hold a non-null `shared_ptr<Map>`.
     * @return the reconstructed aggregate.
     * @throws std::runtime_error if `val` doesn't hold a Map, or the Map is missing a field
     * that `T` declares.
     */
    static T from_value(const Value& val)
    {
        // Must actually hold a live Map — anything else (including a null map
        // pointer) is a hard type mismatch.
        const auto* opt_map = std::get_if<std::shared_ptr<Map>>(&val);
        if ((opt_map == nullptr) || !*opt_map) {
            throw std::runtime_error{std::string{"expected Map for "} + typeid(T).name()};
        }
        T result{};
#ifdef __cpp_reflection
        // Mirror to_value(): walk the same reflected, non-underscore-prefixed
        // fields, pulling each one back out of the map by name.
        const Map& map = **opt_map;
        constexpr auto ctx = std::meta::access_context::current();
        template for (constexpr auto field: std::meta::nonstatic_data_members_of(^^T, ctx))
        {
            constexpr std::string_view name = std::meta::identifier_of(field);
            if (name.starts_with('_')) {
                continue;
            }
            using FT = std::remove_cvref_t<decltype(result.[:field:])>;
            const Value slot = map.get(name);
            if (!slot.has_value()) {
                throw std::runtime_error{std::format("map missing '{}'", name)};
            }
            result.[:field:] = ValueTraits<FT>::from_value(*slot);
        }
#endif
        return result;
    }
};

class AnyConverter
{
public:
    /// @brief Deleted — AnyConverter is a pure static-method utility, never meant to be
    /// instantiated.
    AnyConverter() = delete;

    class HandleTable;

    /**
     * @brief Converts a cross-ABI CongeladoAny into a native Value.
     * @note CG_MAP/CG_ARRAY branches wrap the raw pointer in a non-owning `shared_ptr` (empty
     * deleter) — the pointee's real owner lives elsewhere (typically the HandleTable). Don't
     * let one of these shared_ptrs be the last thing holding a Map/Array alive, it won't be.
     * @param any the cross-ABI value to convert.
     * @return the equivalent native Value; CG_INT and CG_BOOL both map to `Int`, and anything
     * unrecognized falls back to the raw `void *`.
     */
    [[nodiscard]] static Value from_any(const CongeladoAny& any)
    {
        // Dispatch on the cross-ABI type tag — CG_INT and CG_BOOL both collapse
        // down to the same Int variant on this side.
        switch (any.type_index) {
            case CG_NONE:
                return None{};
            case CG_INT:
            case CG_BOOL:
                return Int{any.v_int64};
            case CG_FLOAT:
                return Float{any.v_float64};
            case CG_STR:
                return Str{(any.v_cstr != nullptr) ? std::string{any.v_cstr} : ""};
            // CG_MAP/CG_ARRAY wrap the raw pointer in a non-owning shared_ptr — the
            // real owner lives elsewhere, this is just a borrowing view over it.
            case CG_MAP:
                {
                    auto* raw_ptr = static_cast<Map*>(any.v_ptr);
                    return std::shared_ptr<Map>(raw_ptr, [](Map*) {});
                }
            case CG_ARRAY:
                {
                    auto* raw_ptr = static_cast<Array*>(any.v_ptr);
                    return std::shared_ptr<Array>(raw_ptr, [](Array*) {});
                }
            // Unrecognized type tags fall back to the raw pointer as-is.
            default:
                return any.v_ptr;
        }
    }

    /**
     * @brief Converts a native Value into a cross-ABI CongeladoAny.
     * @warning For `Str`, `any.v_cstr` borrows `value.m_value.c_str()` — that pointer is only
     * valid as long as the `Value` (and its `Str`) it came from stays alive and untouched. For
     * `Map`/`Array`, `any.v_ptr` similarly just borrows the raw pointee — no ownership crosses
     * the ABI boundary here. Hand this CongeladoAny to something that outlives `val` and it's
     * a dangling-pointer L waiting to happen.
     * @param val the Value to convert (taken by non-const ref to match `std::visit`'s needs,
     * though nothing here actually mutates it).
     * @return the equivalent cross-ABI CongeladoAny.
     */
    [[nodiscard]] static CongeladoAny to_any(Value& val)
    {
        CongeladoAny any{};
        // Match the visited alternative against every Value type and fill in the
        // matching CongeladoAny tag + payload — one branch per variant member.
        std::visit(
            [&]<typename T>(T& value) {
                if constexpr (std::same_as<T, None>) {
                    any.type_index = CG_NONE;
                } else if constexpr (std::same_as<T, Bool>) {
                    any.type_index = CG_BOOL;
                    any.v_int64 = value.m_value ? 1 : 0;
                } else if constexpr (std::same_as<T, Int>) {
                    any.type_index = CG_INT;
                    any.v_int64 = value.m_value;
                } else if constexpr (std::same_as<T, Float>) {
                    any.type_index = CG_FLOAT;
                    any.v_float64 = value.m_value;
                } else if constexpr (std::same_as<T, Str>) {
                    any.type_index = CG_STR;
                    any.v_cstr = value.m_value.c_str();
                } else if constexpr (std::same_as<T, std::shared_ptr<Map>>) {
                    any.type_index = CG_MAP;
                    any.v_ptr = value.get();
                } else if constexpr (std::same_as<T, std::shared_ptr<Array>>) {
                    any.type_index = CG_ARRAY;
                    any.v_ptr = value.get();
                } else if constexpr (std::same_as<T, void*>) {
                    any.type_index = CG_PTR;
                    any.v_ptr = value;
                }
            },
            val
        );
        return any;
    }

    /**
     * @brief Converts a native Value into a cross-ABI CongeladoAny without requiring mutable
     * access — functionally identical to to_any(), just taking `val` by const ref.
     * @warning Same borrowing caveats as to_any(): `Str`'s `v_cstr` and `Map`/`Array`'s `v_ptr`
     * both point into `val`'s own storage, no ownership transferred. Outlive `val` with the
     * result and it's dangling — this overload exists for call sites (see HandleTable) that
     * only have a `const Value &` on hand, not because the aliasing risk is any lower.
     * @param val the Value to convert.
     * @return the equivalent cross-ABI CongeladoAny.
     */
    [[nodiscard]] static CongeladoAny to_any_borrow(const Value& val)
    {
        CongeladoAny any{};
        // Same variant-to-tag mapping as to_any() above, just over a const Value —
        // every branch still only borrows into `val`'s own storage.
        std::visit(
            [&]<typename T>(const T& value) {
                if constexpr (std::same_as<T, None>) {
                    any.type_index = CG_NONE;
                } else if constexpr (std::same_as<T, Bool>) {
                    any.type_index = CG_BOOL;
                    any.v_int64 = value.m_value ? 1 : 0;
                } else if constexpr (std::same_as<T, Int>) {
                    any.type_index = CG_INT;
                    any.v_int64 = value.m_value;
                } else if constexpr (std::same_as<T, Float>) {
                    any.type_index = CG_FLOAT;
                    any.v_float64 = value.m_value;
                } else if constexpr (std::same_as<T, Str>) {
                    any.type_index = CG_STR;
                    any.v_cstr = value.m_value.c_str();
                } else if constexpr (std::same_as<T, std::shared_ptr<Map>>) {
                    any.type_index = CG_MAP;
                    any.v_ptr = value.get();
                } else if constexpr (std::same_as<T, std::shared_ptr<Array>>) {
                    any.type_index = CG_ARRAY;
                    any.v_ptr = value.get();
                } else if constexpr (std::same_as<T, void*>) {
                    any.type_index = CG_PTR;
                    any.v_ptr = value;
                }
            },
            val
        );
        return any;
    }
};

class HandleTable
{
public:
    /// @brief Default-constructs an empty HandleTable with no live handles.
    HandleTable() = default;

    /**
     * @brief Inserts a type-erased object and returns a fresh handle id for it.
     * @param obj the object to store, type-erased via `std::any`.
     * @return the newly assigned handle id.
     */
    [[nodiscard]] int64_t insert(std::any obj)
    {
        // Hand out the next id, then store the object under it, bet — ids just
        // count up forever, never reused even after remove().
        auto id = m_next_id++;
        m_handles[id] = std::move(obj);
        return id;
    }

    /**
     * @brief Looks up the type-erased object behind a handle.
     * @param idx the handle id to look up.
     * @return a mutable reference to the stored `std::any`.
     * @throws std::out_of_range if `idx` isn't a live handle.
     */
    [[nodiscard]] std::any& get(int64_t idx)
    {
        return m_handles.at(idx);
    }

    /// @brief Drops a handle, releasing whatever object it referenced.
    /// @param idx the handle id to remove; a no-op if it's not present.
    void remove(int64_t idx)
    {
        m_handles.erase(idx);
    }

    /**
     * @brief Looks up and unwraps the object behind a handle as a specific type.
     * @tparam T the concrete type the handle is expected to hold.
     * @param idx the handle id to look up.
     * @return a copy of the stored object, cast to `T`.
     * @throws std::out_of_range if `idx` isn't a live handle.
     * @throws std::bad_any_cast if the stored object isn't actually a `T`.
     */
    template<typename T>
    [[nodiscard]] T get_as(int64_t idx)
    {
        return std::any_cast<T>(m_handles.at(idx));
    }

    /**
     * @brief Creates a new empty Map and returns a cross-ABI handle to it.
     * @return a CG_MAP_HANDLE-kind CongeladoAny wrapping the new map's handle id.
     */
    [[nodiscard]] CongeladoAny map_create() noexcept
    {
        auto id = insert(std::make_shared<Map>());
        return CongeladoAny{.type_index = CG_MAP_HANDLE, .v_int64 = id};
    }

    // ── Core: simple types ──────────────────────────────────────────────

    /**
     * @brief Sets a key on a map handle to a converted-in cross-ABI value.
     * @param handle handle id of the target map.
     * @param key the entry key.
     * @param value the cross-ABI value to convert and store.
     * @throws std::out_of_range if `handle` isn't a live handle.
     * @throws std::bad_any_cast if `handle` doesn't actually reference a `shared_ptr<Map>`.
     */
    void map_set(int64_t handle, std::string_view key, const CongeladoAny& value)
    {
        auto map = get_as<std::shared_ptr<Map>>(handle);
        // AnyConverter::from_any() only understands the raw-pointer CG_MAP/CG_ARRAY tags, not
        // the handle-id CG_MAP_HANDLE/CG_ARRAY_HANDLE tags returned by
        // map_create()/array_create() — routing those through from_any() fell into its
        // unrecognized-tag fallback and silently stored a null void* instead of the nested
        // container, discovered while testing a nested table round-trip. Resolve the handle id
        // directly instead.
        if (value.type_index == CG_MAP_HANDLE) {
            map->set(std::string{key}, get_as<std::shared_ptr<Map>>(value.v_int64));
            return;
        }
        if (value.type_index == CG_ARRAY_HANDLE) {
            map->set(std::string{key}, get_as<std::shared_ptr<Array>>(value.v_int64));
            return;
        }
        map->set(std::string{key}, AnyConverter::from_any(value));
    }

    /**
     * @brief Gets a map entry by key, converted back out to the cross-ABI form.
     * @param handle handle id of the target map.
     * @param key the entry key to look up.
     * @return the entry's value as a CongeladoAny, or a zero-initialized CongeladoAny if the
     * key isn't present.
     * @throws std::out_of_range if `handle` isn't a live handle.
     * @throws std::bad_any_cast if `handle` doesn't actually reference a `shared_ptr<Map>`.
     */
    [[nodiscard]] CongeladoAny map_get(int64_t handle, std::string_view key)
    {
        auto map = get_as<std::shared_ptr<Map>>(handle);
        // Borrow straight from the Map's own backing storage (get_entries()) rather than
        // through Map::get()'s by-value std::optional<Value> — borrowing into that temporary's
        // owned Value would dangle the instant this function returns.
        const auto& entries = map->get_entries();
        auto it = entries.find(std::string{key});
        // Missing key isn't an error here — just hand back a zeroed CongeladoAny.
        if (it == entries.end()) {
            return CongeladoAny{};
        }
        // to_any_borrow() tags a nested Map/Array as CG_MAP/CG_ARRAY (a raw, non-owning
        // pointer) — but callers that came in through map_set()'s CG_MAP_HANDLE resolution
        // (see above) only understand handle ids, not raw pointers, and have no HandleTable
        // API to operate on one. Discovered via a lua_bridge round-trip test silently losing
        // nested tables. Alias the same shared_ptr under a fresh handle id instead, matching
        // get_map_keys()'s existing fresh-handle pattern; the caller is expected to
        // handle_free() it once done, same as any other handle this table hands out.
        if (auto* nested_map = std::get_if<std::shared_ptr<Map>>(&it->second)) {
            return CongeladoAny{.type_index = CG_MAP_HANDLE, .v_int64 = insert(*nested_map)};
        }
        if (auto* nested_array = std::get_if<std::shared_ptr<Array>>(&it->second)) {
            return CongeladoAny{.type_index = CG_ARRAY_HANDLE, .v_int64 = insert(*nested_array)};
        }
        return AnyConverter::to_any_borrow(it->second);
    }

    /**
     * @brief Gets how many entries a map handle currently holds.
     * @param handle handle id of the target map.
     * @return a CG_INT-kind CongeladoAny wrapping the entry count.
     * @throws std::out_of_range if `handle` isn't a live handle.
     * @throws std::bad_any_cast if `handle` doesn't actually reference a `shared_ptr<Map>`.
     */
    [[nodiscard]] CongeladoAny get_map_size(int64_t handle)
    {
        auto map = get_as<std::shared_ptr<Map>>(handle);
        return CongeladoAny{.type_index = CG_INT, .v_int64 = static_cast<int64_t>(map->get_size())};
    }

    /**
     * @brief Builds a fresh array of a map's keys and returns a handle to it.
     * @note The caller owns the returned array handle and is expected to handle_free() it —
     * this is a brand-new Array, not a view into the map, so it's on the caller not to sleep
     * on releasing it.
     * @param handle handle id of the target map.
     * @return a CG_ARRAY_HANDLE-kind CongeladoAny wrapping the new keys array's handle id.
     * @throws std::out_of_range if `handle` isn't a live handle.
     * @throws std::bad_any_cast if `handle` doesn't actually reference a `shared_ptr<Map>`.
     */
    [[nodiscard]] CongeladoAny get_map_keys(int64_t handle)
    {
        auto map = get_as<std::shared_ptr<Map>>(handle);
        // Build a brand-new Array of Str keys, lowkey a snapshot copy, not a
        // view into the map, so the caller is expected to handle_free() it later.
        auto keys_array = std::make_shared<Array>();
        for (const auto& key: map->get_entries() | std::views::keys) {
            (*keys_array).push(Str{key});
        }
        auto id = insert(std::move(keys_array));
        return CongeladoAny{.type_index = CG_ARRAY_HANDLE, .v_int64 = id};
    }

    /// @brief Frees a handle (map or array), releasing whatever it referenced.
    /// @param handle the handle id to free; a no-op if it's not present.
    void handle_free(int64_t handle) noexcept
    {
        remove(handle);
    }

    /**
     * @brief Creates a new empty Array (optionally pre-sized) and returns a cross-ABI handle to
     * it.
     * @param capacity optional item count to reserve up front; defaults to no reservation.
     * @return a CG_ARRAY_HANDLE-kind CongeladoAny wrapping the new array's handle id.
     */
    [[nodiscard]] CongeladoAny array_create(std::optional<int64_t> capacity = std::nullopt)
    {
        auto array_size = capacity.has_value() ? static_cast<std::size_t>(capacity.value())
                                               : static_cast<std::size_t>(0);
        auto id = insert(std::make_shared<Array>(array_size));
        return CongeladoAny{.type_index = CG_ARRAY_HANDLE, .v_int64 = id};
    }

    /**
     * @brief Appends a converted-in cross-ABI value to an array handle.
     * @param handle handle id of the target array.
     * @param value the cross-ABI value to convert and append.
     * @throws std::out_of_range if `handle` isn't a live handle.
     * @throws std::bad_any_cast if `handle` doesn't actually reference a `shared_ptr<Array>`.
     */
    void array_push(int64_t handle, const CongeladoAny& value)
    {
        auto arr = get_as<std::shared_ptr<Array>>(handle);
        // Same handle-id-vs-raw-pointer tag mismatch as map_set() above — resolve
        // CG_MAP_HANDLE/CG_ARRAY_HANDLE directly instead of routing through from_any().
        if (value.type_index == CG_MAP_HANDLE) {
            arr->push(get_as<std::shared_ptr<Map>>(value.v_int64));
            return;
        }
        if (value.type_index == CG_ARRAY_HANDLE) {
            arr->push(get_as<std::shared_ptr<Array>>(value.v_int64));
            return;
        }
        arr->push(AnyConverter::from_any(value));
    }

    /**
     * @brief Gets how many items an array handle currently holds.
     * @param handle handle id of the target array.
     * @return a CG_INT-kind CongeladoAny wrapping the item count.
     * @throws std::out_of_range if `handle` isn't a live handle.
     * @throws std::bad_any_cast if `handle` doesn't actually reference a `shared_ptr<Array>`.
     */
    [[nodiscard]] CongeladoAny get_array_size(int64_t handle)
    {
        auto arr = get_as<std::shared_ptr<Array>>(handle);
        return CongeladoAny{.type_index = CG_INT, .v_int64 = static_cast<int64_t>(arr->get_size())};
    }

    /**
     * @brief Gets an array item by index, converted back out to the cross-ABI form.
     * @param handle handle id of the target array.
     * @param index zero-based item index to look up.
     * @return the item's value as a CongeladoAny, or a zero-initialized CongeladoAny if
     * `index` is out of range.
     * @throws std::out_of_range if `handle` isn't a live handle.
     * @throws std::bad_any_cast if `handle` doesn't actually reference a `shared_ptr<Array>`.
     */
    [[nodiscard]] CongeladoAny array_get(int64_t handle, int64_t index)
    {
        auto arr = get_as<std::shared_ptr<Array>>(handle);
        // Borrow straight from the Array's own backing storage (get_items()) rather than
        // through Array::get()'s by-value std::optional<Value> — same dangling-temporary
        // reasoning as map_get() above.
        auto items = arr->get_items();
        // Out-of-range index isn't an error here either — same zeroed fallback
        // pattern as map_get().
        if (index < 0 || static_cast<std::size_t>(index) >= items.size()) {
            return CongeladoAny{};
        }
        // Same handle-id-vs-raw-pointer fix as map_get() above.
        auto& item = items[static_cast<std::size_t>(index)];
        if (auto* nested_map = std::get_if<std::shared_ptr<Map>>(&item)) {
            return CongeladoAny{.type_index = CG_MAP_HANDLE, .v_int64 = insert(*nested_map)};
        }
        if (auto* nested_array = std::get_if<std::shared_ptr<Array>>(&item)) {
            return CongeladoAny{.type_index = CG_ARRAY_HANDLE, .v_int64 = insert(*nested_array)};
        }
        return AnyConverter::to_any_borrow(item);
    }

    // ── const char* key overloads ───────────────────────────────────────

    /**
     * @brief `const char *` convenience overload of map_set() for raw C-string keys.
     * @param handle handle id of the target map.
     * @param key C-string key; treated as an empty key if null.
     * @param value the cross-ABI value to convert and store.
     */
    void map_set(int64_t handle, const char* key, const CongeladoAny& value)
    {
        map_set(handle, std::string_view{(key != nullptr) ? key : ""}, value);
    }

    /**
     * @brief `const char *` convenience overload of map_get() for raw C-string keys.
     * @param handle handle id of the target map.
     * @param key C-string key; treated as an empty key if null.
     * @return the entry's value as a CongeladoAny, or a zero-initialized CongeladoAny if
     * the key isn't present.
     */
    [[nodiscard]] CongeladoAny map_get(int64_t handle, const char* key)
    {
        return map_get(handle, std::string_view{(key != nullptr) ? key : ""});
    }

    // ── Backward-compat overloads (CongeladoAny*) ──────────────────────

    /**
     * @brief Pointer-args backward-compat overload of map_set(), for callers passing
     * everything as `CongeladoAny *` across the C ABI.
     * @warning `handler`, `key`, and `value` are all dereferenced with zero null checks —
     * unlike the `const char *` overloads above, which do guard against null. Pass a null
     * `handler`/`key`/`value` here and it's an immediate crash, no cap.
     * @param handler CongeladoAny whose `v_int64` is the target map's handle id.
     * @param key CongeladoAny whose `v_cstr` is the entry key; treated as empty if null.
     * @param value the cross-ABI value to convert and store.
     */
    void map_set(const CongeladoAny* handler, const CongeladoAny* key, const CongeladoAny* value)
    {
        map_set(
            handler->v_int64, (key->v_cstr != nullptr) ? std::string_view{key->v_cstr} : "", *value
        );
    }

    /**
     * @brief Pointer-args backward-compat overload of map_get().
     * @warning `handler` and `key` are dereferenced with no null check on `handler` itself.
     * @param handler CongeladoAny whose `v_int64` is the target map's handle id.
     * @param key CongeladoAny whose `v_cstr` is the entry key; treated as empty if null.
     * @return the entry's value as a CongeladoAny, or a zero-initialized CongeladoAny if
     * the key isn't present.
     */
    [[nodiscard]] CongeladoAny map_get(const CongeladoAny* handler, const CongeladoAny* key)
    {
        return map_get(
            handler->v_int64, (key->v_cstr != nullptr) ? std::string_view{key->v_cstr} : ""
        );
    }

    /**
     * @brief Pointer-args backward-compat overload of get_map_size().
     * @param handler CongeladoAny whose `v_int64` is the target map's handle id.
     * @return a CG_INT-kind CongeladoAny wrapping the entry count.
     */
    [[nodiscard]] CongeladoAny get_map_size(const CongeladoAny* handler)
    {
        return get_map_size(handler->v_int64);
    }

    /**
     * @brief Pointer-args backward-compat overload of get_map_keys().
     * @param handler CongeladoAny whose `v_int64` is the target map's handle id.
     * @return a CG_ARRAY_HANDLE-kind CongeladoAny wrapping the new keys array's handle id.
     */
    [[nodiscard]] CongeladoAny get_map_keys(const CongeladoAny* handler)
    {
        return get_map_keys(handler->v_int64);
    }

    /// @brief Pointer-args backward-compat overload of handle_free().
    /// @param handler CongeladoAny whose `v_int64` is the handle id to free.
    void handle_free(const CongeladoAny* handler) noexcept
    {
        handle_free(handler->v_int64);
    }

    /**
     * @brief Pointer-args backward-compat overload of array_create().
     * @param cap optional CongeladoAny giving the reserve capacity; only honored when
     * non-null and CG_INT-kind, otherwise falls back to the no-reservation default.
     * @return a CG_ARRAY_HANDLE-kind CongeladoAny wrapping the new array's handle id.
     */
    [[nodiscard]] CongeladoAny array_create(const CongeladoAny* cap)
    {
        // Only honor the capacity hint if it's actually present and CG_INT-kind —
        // anything else falls back to the no-reservation default overload.
        if (cap != nullptr && cap->type_index == CG_INT) {
            return array_create(static_cast<int64_t>(cap->v_int64));
        }
        return array_create();
    }

    /**
     * @brief Pointer-args backward-compat overload of array_push().
     * @warning `handler` and `value` are dereferenced with no null check.
     * @param handler CongeladoAny whose `v_int64` is the target array's handle id.
     * @param value the cross-ABI value to convert and append.
     */
    void array_push(const CongeladoAny* handler, const CongeladoAny* value)
    {
        array_push(handler->v_int64, *value);
    }

    /**
     * @brief Pointer-args backward-compat overload of get_array_size().
     * @param handler CongeladoAny whose `v_int64` is the target array's handle id.
     * @return a CG_INT-kind CongeladoAny wrapping the item count.
     */
    [[nodiscard]] CongeladoAny get_array_size(const CongeladoAny* handler)
    {
        return get_array_size(handler->v_int64);
    }

    /**
     * @brief Pointer-args backward-compat overload of array_get().
     * @param handler CongeladoAny whose `v_int64` is the target array's handle id.
     * @param index optional CongeladoAny giving the item index; only honored when non-null
     * and CG_INT-kind, otherwise defaults to index 0.
     * @return the item's value as a CongeladoAny, or a zero-initialized CongeladoAny if the
     * resolved index is out of range.
     */
    [[nodiscard]] CongeladoAny array_get(const CongeladoAny* handler, const CongeladoAny* index)
    {
        // Same "only honor it if it's really there and CG_INT-kind" pattern as
        // array_create() — otherwise just default to index 0.
        std::size_t idx = (index != nullptr && index->type_index == CG_INT)
                              ? static_cast<std::size_t>(index->v_int64)
                              : 0;
        return array_get(handler->v_int64, static_cast<int64_t>(idx));
    }

private:
    std::unordered_map<int64_t, std::any> m_handles;
    int64_t m_next_id = 1;
};

} // namespace core::plugin

#ifdef CONGELADO_TEST
namespace core::plugin::tests {
using namespace boost::ut;

suite<"Map"> map_suite = [] {
    "starts empty"_test = [] {
        Map map;
        expect(map.get_size() == 0);
        expect(not map.get("missing").has_value());
    };
    "set then get round-trips a value, overwriting on repeat set"_test = [] {
        Map map;
        map.set("a", Int{1});
        map.set("a", Int{2});

        expect(map.get_size() == 1);
        auto val = map.get("a");
        expect(val.has_value());
        expect(std::get<Int>(*val).m_value == 2);
    };
};

suite<"Array"> array_suite = [] {
    "starts empty"_test = [] {
        Array arr;
        expect(arr.get_size() == 0);
        expect(not arr.get(0).has_value());
    };
    "push appends, get retrieves by index"_test = [] {
        Array arr;
        arr.push(Int{10});
        arr.push(Str{"x"});

        expect(arr.get_size() == 2);
        expect(std::get<Int>(*arr.get(0)).m_value == 10);
        expect(std::get<Str>(*arr.get(1)).m_value == "x");
        expect(not arr.get(2).has_value());
    };
};

suite<"ValueTraits<bool>"> value_traits_bool_suite = [] {
    "to_value wraps as Bool"_test = [] {
        auto val = ValueTraits<bool>::to_value(true);
        expect(std::get<Bool>(val).m_value);
    };
    "from_value accepts Bool directly and Int as nonzero-coercion"_test = [] {
        expect(ValueTraits<bool>::from_value(Bool{true}));
        expect(not ValueTraits<bool>::from_value(Bool{false}));
        expect(ValueTraits<bool>::from_value(Int{5}));
        expect(not ValueTraits<bool>::from_value(Int{0}));
    };
    "from_value throws for anything else"_test = [] {
        expect(throws<std::runtime_error>([] {
            ValueTraits<bool>::from_value(Str{"x"});
        }));
    };
};

suite<"ValueTraits<integral/floating> to_value"> value_traits_numeric_to_value_suite = [] {
    // from_value on these two specializations is documented dead/broken code (missing `static`,
    // wrong member name) that can't be called without a hard compile error — see the
    // @warning on ValueTraits<T>::from_value in this file. to_value is unaffected and correct.
    "integral to_value wraps as Int"_test = [] {
        expect(std::get<Int>(ValueTraits<int>::to_value(42)).m_value == 42);
    };
    "floating-point to_value wraps as Float"_test = [] {
        expect(std::get<Float>(ValueTraits<double>::to_value(1.5)).m_value == 1.5);
    };
};

suite<"ValueTraits<std::string>"> value_traits_string_suite = [] {
    "to_value/from_value round-trip"_test = [] {
        auto val = ValueTraits<std::string>::to_value("hello");
        expect(ValueTraits<std::string>::from_value(val) == "hello");
    };
    "from_value throws for a non-Str value"_test = [] {
        expect(throws<std::runtime_error>([] {
            ValueTraits<std::string>::from_value(Int{1});
        }));
    };
};

suite<"AnyConverter"> any_converter_suite = [] {
    "from_any maps every scalar tag to the matching Value alternative"_test = [] {
        expect(
            std::holds_alternative<None>(
                AnyConverter::from_any(CongeladoAny{.type_index = CG_NONE})
            )
        );
        expect(
            std::get<Int>(AnyConverter::from_any(CongeladoAny{.type_index = CG_INT, .v_int64 = 7}))
                .m_value == 7
        );
        expect(
            std::get<Float>(
                AnyConverter::from_any(CongeladoAny{.type_index = CG_FLOAT, .v_float64 = 2.5})
            )
                .m_value == 2.5
        );
    };
    "to_any round-trips an Int Value back to a CongeladoAny"_test = [] {
        Value val = Int{99};
        auto any = AnyConverter::to_any(val);
        expect(any.type_index == CG_INT);
        expect(any.v_int64 == 99);
    };
};

// NOTE(findings): the CongeladoAny*-taking overloads of map_set/map_get/array_push/array_get/
// handle_free (~line 731-823) dereference handler/key/value with zero null checks — a null
// CongeladoAny* crashes the process immediately. Not exercised here per the "never trigger real
// memory corruption/crash" safety constraint; comment-only.
suite<"HandleTable"> handle_table_suite = [] {
    "map_create/map_set/map_get/get_map_size round-trip"_test = [] {
        HandleTable table;
        auto handle = table.map_create();
        expect(handle.type_index == CG_MAP_HANDLE);

        table.map_set(handle.v_int64, "key", CongeladoAny{.type_index = CG_INT, .v_int64 = 42});
        auto result = table.map_get(handle.v_int64, "key");
        expect(result.type_index == CG_INT);
        expect(result.v_int64 == 42);

        auto size = table.get_map_size(handle.v_int64);
        expect(size.v_int64 == 1);
    };
    "map_get on a missing key returns a zeroed CongeladoAny"_test = [] {
        HandleTable table;
        auto handle = table.map_create();
        auto result = table.map_get(handle.v_int64, "missing");
        expect(result.type_index == CG_NONE);
    };
    "array_create/array_push/array_get/get_array_size round-trip"_test = [] {
        HandleTable table;
        auto handle = table.array_create();
        table.array_push(handle.v_int64, CongeladoAny{.type_index = CG_INT, .v_int64 = 1});
        table.array_push(handle.v_int64, CongeladoAny{.type_index = CG_INT, .v_int64 = 2});

        expect(table.get_array_size(handle.v_int64).v_int64 == 2);
        expect(table.array_get(handle.v_int64, 0).v_int64 == 1);
        expect(table.array_get(handle.v_int64, 1).v_int64 == 2);
    };
    "handle_free drops the handle, later access throws"_test = [] {
        HandleTable table;
        auto handle = table.map_create();
        table.handle_free(handle.v_int64);
        expect(throws<std::out_of_range>([&] {
            [[maybe_unused]] auto result = table.get_map_size(handle.v_int64);
        }));
    };
    "get_as throws bad_any_cast for a mismatched handle type"_test = [] {
        HandleTable table;
        auto map_handle = table.map_create();
        expect(throws<std::bad_any_cast>([&] {
            [[maybe_unused]] auto result = table.get_array_size(map_handle.v_int64);
        }));
    };
};

} // namespace core::plugin::tests
#endif
// NOLINTEND(cppcoreguidelines-pro-type-union-access)
