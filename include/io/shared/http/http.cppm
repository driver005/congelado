export module io_shared:http;

export import :http_header;
export import :http_res;
export import :http_req;
export import :http_types;

import std;

export namespace io::shared::http {

using Dispatcher = std::function<HttpResponse(HttpRequest &)>;

} // namespace io::shared::http
