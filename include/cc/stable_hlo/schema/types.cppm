module;

export module cc_stable_hlo:types;

import std;

export namespace cc::stable_hlo {

// Plain data — the unbound schema shapes (describe an op/param/attr KIND, not a bound value).
// Grouped in one file since none of the three carries behavior beyond holding fields; the
// one-class-per-file rule is about substantial classes, not trivially-coupled data structs.

struct StableHloParamSchema {
    std::string name;
    bool variadic{false};
};

struct StableHloAttrSchema {
    std::string name;
    std::string cpp_type;
    bool optional{false};
    bool list{false};
};

struct StableHloOpSchema {
    std::string name;
    std::string summary;
    std::string category;
    std::vector<StableHloParamSchema> inputs;
    std::vector<StableHloAttrSchema> attrs;
    int output_count{1};
};

} // namespace cc::stable_hlo
