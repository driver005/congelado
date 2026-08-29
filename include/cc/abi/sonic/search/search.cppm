module;

#include "c/extern/search/search.h"

export module cc_abi_sonic_search;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Runtime — the mainframe-facing search handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator. The cross-plugin path converts
// ice::SearchQuery (pure C++) into a stack TF_Search_Query (the raw C-ABI
// value struct) right at the call site — the query never needs to be owned/kept alive past the
// call.
class Search : public ice::sonic::Runtime<Search, TF_Search>
{
public:
    explicit Search(TF_Search* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "search";

    [[nodiscard]] std::expected<void, ice::Status> index(
        const ice::String& collection,
        const ice::String& id,
        const ice::String& document_json,
        TF_Search_CompletionFn completion,
        void* cb_user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->index(
            get_handle(),
            collection.get_handle(),
            id.get_handle(),
            document_json.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> remove(
        const ice::String& collection,
        const ice::String& id,
        TF_Search_CompletionFn completion,
        void* cb_user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->remove(
            get_handle(),
            collection.get_handle(),
            id.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> search(
        const ice::String& collection,
        const ice::SearchQuery& query,
        TF_Search_CompletionFn completion,
        void* cb_user_data
    ) noexcept
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

        m_ops->search(
            get_handle(),
            collection.get_handle(),
            &tf_query,
            completion,
            cb_user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }
};

} // namespace ice::sonic
