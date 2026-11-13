export module interfaces:cache;

import std;
import shared;

export namespace interfaces {

class ICache
{
public:
    /// @brief Default ctor — kept explicit since declaring the copy/move members below would
    /// otherwise suppress it, and derived backends (e.g. LocalCache) rely on default
    /// constructing this base.
    ICache() = default;

    /**
     * @brief Virtual dtor, default's straight bet — any concrete cache backend gets cleaned up
     * through the base pointer, no drama, no leaks, no cap.
     */
    virtual ~ICache() = default;

    /**
     * @brief Copy ctor, defaulted — no data members of its own, so member-wise copy is
     * trivially correct.
     */
    ICache(const ICache&) = default;
    /**
     * @brief Copy assignment, defaulted alongside the copy ctor for the same reason.
     */
    ICache& operator=(const ICache&) = default;
    /**
     * @brief Move ctor, defaulted — same story, nothing owned that needs special handling.
     */
    ICache(ICache&&) = default;
    /**
     * @brief Move assignment, defaulted to round out the set.
     */
    ICache& operator=(ICache&&) = default;

    /**
     * @brief Tells you which cache backend you're actually vibing with right now (redis,
     * memcached, whatever's plugged in) — lowkey the whole identity of this thing.
     * @return the backend's name, straight from the implementer, no filter.
     */
    [[nodiscard]] virtual std::string_view backend_name() const noexcept = 0;

    /**
     * @brief Fires off an async lookup for `key` — zero blocking, the result just shows up
     * through the callback whenever the backend's actually done cooking.
     * @param key the cache key you're trying to pull.
     * @param result callback that gets handed the read outcome once it lands.
     */
    virtual void get(std::string_view key, shared::QueryReadFn&& result) noexcept = 0;
    /**
     * @brief Writes `value` under `key`, async style, no blocking — reports back through
     * `result` once the write actually goes through.
     * @param key the cache key to write to.
     * @param value the value getting stashed.
     * @param result callback that gets the write outcome, W or L.
     */
    virtual void
    set(std::string_view key, std::string_view value, shared::QueryReadFn&& result) noexcept = 0;
    /**
     * @brief Yeets whatever's stored under `key` straight out of the cache, async, no cap,
     * reports back through `result` when it's actually gone.
     * @param key the cache key to remove.
     * @param result callback that gets the removal outcome.
     */
    virtual void remove(std::string_view key, shared::QueryReadFn&& result) noexcept = 0;
};

} // namespace interfaces
