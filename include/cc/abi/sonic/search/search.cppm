module;

#include "c/extern/search/search.h"

export module cc_abi_sonic_search;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;

namespace ice::sonic::detail {
inline TF_Search_CompletionFn to_c(ice::builder::CompletionFn fn) noexcept {
    static_assert(sizeof(ice::builder::CompletionFn) == sizeof(TF_Search_CompletionFn));
    return std::bit_cast<TF_Search_CompletionFn>(fn);
}
} // namespace ice::sonic::detail
export namespace ice::sonic {

// Runtime — the mainframe-facing search handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator. The cross-plugin path converts
// ice::builder::SearchQuery (pure C++) into a stack TF_Search_Query (the raw C-ABI
// value struct) right at the call site — the query never needs to be owned/kept alive past the
// call, unlike the async completion contexts elsewhere in this file.
class Search : public ice::sonic::Runtime<Search, TF_Search, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "search";

    std::expected<void, ice::Status> index(
        const ice::String& collection,
        const ice::String& id,
        const ice::String& document_json,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->index(this->get_handle(), collection.get_handle(), id.get_handle(),
            document_json.get_handle(), detail::to_c(completion), cb_user_data, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> remove(
        const ice::String& collection,
        const ice::String& id,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;
        this->m_ops->remove(this->get_handle(), collection.get_handle(), id.get_handle(),
            detail::to_c(completion), cb_user_data, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    std::expected<void, ice::Status> search(
        const ice::String& collection,
        const ice::builder::SearchQuery& query,
        ice::builder::CompletionFn completion,
        void* cb_user_data
    )
    {
        ice::Status status;

        ice::String sr_query(query.get_query());
        ice::String sr_free_text(query.get_free_text());
        ice::String sr_sort(query.get_sort());

        TF_Search_Query tf_query;
        tf_query.struct_size = TF_SEARCH_QUERY_STRUCT_SIZE;
        tf_query.ext = nullptr;
        tf_query.query = *sr_query.get_handle();
        tf_query.free_text = *sr_free_text.get_handle();
        tf_query.start = query.get_start();
        tf_query.size = query.get_size();
        tf_query.sort = *sr_sort.get_handle();

        this->m_ops->search(this->get_handle(), collection.get_handle(), &tf_query,
            detail::to_c(completion), cb_user_data, status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const
    {
        ice::String tf_name;
        this->m_ops->get_name(this->get_handle(), tf_name.get_handle());
        return std::move(tf_name);
    }

public:
    explicit Search(TF_Search* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
