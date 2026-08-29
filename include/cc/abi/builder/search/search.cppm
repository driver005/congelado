module;

#include "c/extern/search/search.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_search;

export import :search_query;
import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import :search_query;

export namespace ice::builder {

using CompletionFn = void (*)(
    const ice::String& result,
    void* user_data
);

inline CompletionFn completion_from_c(TF_Search_CompletionFn fn) noexcept
{
    static_assert(sizeof(CompletionFn) == sizeof(TF_Search_CompletionFn));
    return std::bit_cast<CompletionFn>(fn);
}

inline TF_Search_CompletionFn completion_to_c(CompletionFn fn) noexcept
{
    static_assert(sizeof(TF_Search_CompletionFn) == sizeof(CompletionFn));
    return std::bit_cast<TF_Search_CompletionFn>(fn);
}

// Abstract base class for a search backend — pure interface, zero C-ABI/TF_*
// knowledge, mirrors ice::builder::Builder's role. A backend implements this
// directly and registers a factory function pointer into
// ice::sonic::RegistrationRuntime under type="search".
class Search
{
public:
    virtual ~Search() = default;

    virtual std::expected<
        void,
        ice::Status>
    index(
        const ice::String& collection,
        const ice::String& id,
        const ice::String& document_json,
        CompletionFn completion,
        void* cb_user_data
    ) = 0;

    virtual std::expected<
        void,
        ice::Status>
    remove(
        const ice::String& collection,
        const ice::String& id,
        CompletionFn completion,
        void* cb_user_data
    ) = 0;

    virtual std::expected<
        void,
        ice::Status>
    search(
        const ice::String& collection,
        const SearchQuery& query,
        CompletionFn completion,
        void* cb_user_data
    ) = 0;

    virtual ice::String get_name() const = 0;

    TF_Search* get_generic_vtable()
    {
        static TF_Search vtable = {
            .struct_size = sizeof(TF_Search),
            .destroy =
                [](void* ctx) {
                    delete ctx_as<Search>(ctx);
                },
            .get_name =
                [](void* ctx, TF_String* out) {
                    auto* self = ctx_as<Search>(ctx);
                    auto name = self->get_name();
                    name.to_c(out);
                },
            .index =
                [](void* ctx, const TF_TString* collection, const TF_TString* id,
                   const TF_TString* document_json, TF_Search_CompletionFn completion,
                   void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Search>(ctx);
                    auto res = self->index(
                        ice::String::create(collection), ice::String::create(id),
                        ice::String::create(document_json), completion_from_c(completion),
                        cb_user_data
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                },
            .remove =
                [](void* ctx, const TF_TString* collection, const TF_TString* id,
                   TF_Search_CompletionFn completion, void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Search>(ctx);
                    auto res = self->remove(
                        ice::String::create(collection), ice::String::create(id),
                        completion_from_c(completion), cb_user_data
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                },
            .search =
                [](void* ctx, const TF_TString* collection, const TF_Search_Query* query,
                   TF_Search_CompletionFn completion, void* cb_user_data, TF_Status* status) {
                    auto* self = ctx_as<Search>(ctx);
                    SearchQuery cpp_query;
                    cpp_query.set_query(ice::String::create(&query->query))
                        .set_free_text(ice::String::create(&query->free_text))
                        .set_start(query->start)
                        .set_size(query->size)
                        .set_sort(ice::String::create(&query->sort));
                    auto res = self->search(
                        ice::String::create(collection), cpp_query, completion_from_c(completion),
                        cb_user_data
                    );
                    if (!res) {
                        res.error().to_c(status);
                    }
                }
        };
        return &vtable;
    }
};

} // namespace ice::builder
