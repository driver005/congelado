module;

#include "c/extern/search/search.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_search;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

// Abstract base class for a search backend — pure interface, zero C-ABI/TF_*
// knowledge, mirrors ice::builder::Generator's role. A backend implements this
// directly and registers a factory function pointer into
// ice::sonic::RegistrationRuntime under type="search".
class Search
{
public:
    // Recover the Search instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Search* create(void* ctx) noexcept
    {
        return static_cast<Search*>(ctx);
    }

    virtual ~Search() = default;

    [[nodiscard]] virtual std::expected<void, ice::Status> index(
        const ice::String& collection,
        const ice::String& id,
        const ice::String& document_json,
        TF_Search_CompletionFn completion,
        void* cb_user_data
    ) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status> remove(
        const ice::String& collection,
        const ice::String& id,
        TF_Search_CompletionFn completion,
        void* cb_user_data
    ) noexcept = 0;

    [[nodiscard]] virtual std::expected<void, ice::Status> search(
        const ice::String& collection,
        const ice::SearchQuery& query,
        TF_Search_CompletionFn completion,
        void* cb_user_data
    ) noexcept = 0;

    virtual ice::String get_name() const noexcept = 0;

    static TF_Search* get_generic_vtable()
    {
        static TF_Search vtable = {
            .struct_size = TF_SEARCH_STRUCT_SIZE,
            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Search::create(plugin_context);
            },
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Search::create(plugin_context);
                auto name = self->get_name();
                name.to_c(out);
            },
            .index =
                [](void* plugin_context,
                   const TF_TString* collection,
                   const TF_TString* id,
                   const TF_TString* document_json,
                   TF_Search_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status) noexcept
            {
                auto* self = Search::create(plugin_context);
                auto res = self->index(
                    ice::String::create(collection),
                    ice::String::create(id),
                    ice::String::create(document_json),
                    completion,
                    cb_user_data
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .remove =
                [](void* plugin_context,
                   const TF_TString* collection,
                   const TF_TString* id,
                   TF_Search_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status) noexcept
            {
                auto* self = Search::create(plugin_context);
                auto res = self->remove(
                    ice::String::create(collection),
                    ice::String::create(id),
                    completion,
                    cb_user_data
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .search =
                [](void* plugin_context,
                   const TF_TString* collection,
                   const TF_Search_Query* query,
                   TF_Search_CompletionFn completion,
                   void* cb_user_data,
                   TF_Status* status) noexcept
            {
                auto* self = Search::create(plugin_context);
                ice::SearchQuery cpp_query;
                cpp_query.set_query(ice::String::create(&query->query))
                    .set_free_text(ice::String::create(&query->free_text))
                    .set_start(query->start)
                    .set_size(query->size)
                    .set_sort(ice::String::create(&query->sort));
                auto res = self->search(
                    ice::String::create(collection),
                    cpp_query,
                    completion,
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
