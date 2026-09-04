export module cc_abi_gen_generator:type_registry;

import std;
import :known_type;

export namespace cc_abi_gen {

// Every type this generator knows how to wrap across the C ABI boundary (the "known" list),
// plus every type some domain's header referenced but that isn't known yet (the "pending"
// list). A type leaves "pending" only when its own domain is actually generated and registered
// — if anything is still pending once every requested domain has been processed, that's a real
// modeling gap: silently passing an unmodeled type through unconverted would be wrong, not just
// incomplete, so the caller must fail the run rather than emit something quietly broken.
class TypeRegistry
{
public:
    TypeRegistry()
    {
        // TF_TString / its TF_String alias — the one wrap kind verified against the real
        // hand-written cache.cppm/logger.cppm: a zero-copy view on the way in
        // (ice::String::create), get_handle() on the way out.
        KnownType tstring;
        tstring.m_pointee_name = "TF_TString";
        tstring.m_cpp_parameter_type = "const ice::String &";
        tstring.m_wrap_format = "ice::String::create({})";
        tstring.m_unwrap_format = "{}.get_handle()";
        m_known_types.push_back(tstring);

        KnownType string_alias = tstring;
        string_alias.m_pointee_name = "TF_String";
        m_known_types.push_back(string_alias);

        // TF_Status is never itself a wrapped parameter in this generator's output — it's the
        // fallibility marker (SlotClassifier::is_fallible), always the trailing parameter,
        // detected structurally rather than through this table. Registered anyway so a domain
        // that happens to reference it in a non-trailing position is recognized as known rather
        // than flagged as an unmodeled gap.
        KnownType status;
        status.m_pointee_name = "TF_Status";
        status.m_cpp_parameter_type = "ice::Status &";
        status.m_unwrap_format = "{}.get_handle()";
        m_known_types.push_back(status);
    }

    const KnownType *find(const std::string &pointee_name) const
    {
        for (const KnownType &entry : m_known_types) {

            if (entry.m_pointee_name == pointee_name) {

                return &entry;
            }
        }

        return nullptr;
    }

    // Called when a domain's header references a type this registry doesn't know how to wrap
    // yet. No-op if it's already known or already pending.
    void note_unknown(const std::string &pointee_name)
    {
        if (find(pointee_name) != nullptr) {

            return;
        }

        for (const std::string &pending : m_pending_types) {

            if (pending == pointee_name) {

                return;
            }
        }

        m_pending_types.push_back(pointee_name);
    }

    // Called once a domain has actually been generated: registers its type and clears it from
    // pending if some earlier domain had already referenced it.
    void register_generated(const KnownType &entry)
    {
        m_known_types.push_back(entry);

        std::erase(m_pending_types, entry.m_pointee_name);
    }

    bool has_pending() const
    {
        return !m_pending_types.empty();
    }

    const std::vector<std::string> &pending() const
    {
        return m_pending_types;
    }

private:
    std::vector<KnownType> m_known_types;
    std::vector<std::string> m_pending_types;
};

} // namespace cc_abi_gen
