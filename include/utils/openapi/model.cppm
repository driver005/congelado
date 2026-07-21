export module utils_openapi:model;

import std;
import serde;

export namespace utils::openapi {

class SchemaObject {
  public:
    /**
     * @brief Default ctor — blank schema, no type/format/ref set yet. Fill it in with the
     * setters below, that's the whole motion.
     */
    SchemaObject() = default;

    /**
     * @brief Sets the JSON Schema "type" keyword (e.g. "string", "object", "array").
     * @param value the type string to store.
     */
    void set_type(std::string value) { m_type = std::move(value); }
    /**
     * @brief Sets the JSON Schema "format" keyword (e.g. "int64", "date-time") — extra type
     * hint layered on top of get_type(), purely descriptive, nothing here validates it, no cap.
     * @param value the format string to store.
     */
    void set_format(std::string value) { m_format = std::move(value); }
    /**
     * @brief Flags whether this schema allows null, straight up.
     * @param value true if null's a valid value here, false otherwise.
     */
    void set_nullable(bool value) noexcept { m_nullable = value; }
    /**
     * @brief Sets the "$ref" pointer to a named schema under components.schemas — this is how
     * build_schema<T>() links a serializable type back to its registered definition instead of
     * inlining the whole thing twice. No cap, this is the dedup motion.
     * @param value the $ref string, e.g. "#/components/schemas/TaskDef".
     */
    void set_ref(std::string value) { m_ref = std::move(value); }
    /**
     * @brief Appends a property name to the "required" list.
     * @param name the required property's name.
     */
    void add_required(std::string name) { m_required.push_back(std::move(name)); }
    /**
     * @brief Appends one more allowed value to the "enum" list.
     * @param value the enum value to add.
     */
    void add_enum_value(std::string value) { m_enum_values.push_back(std::move(value)); }
    /**
     * @brief Adds a named property schema — heap-allocates it into a shared_ptr since
     * SchemaObject nests itself recursively and a plain by-value member here would never stop
     * growing. Standard recursive-type motion.
     * @param name the property's name.
     * @param schema the property's schema, moved in and wrapped.
     */
    void add_property(std::string name, SchemaObject schema) {
        m_properties.emplace(std::move(name), std::make_shared<SchemaObject>(std::move(schema)));
    }
    /**
     * @brief Sets the array element schema (only meaningful when get_type() == "array") — same
     * shared_ptr-wrapping trick as add_property(), for the same recursive-type reason.
     * @param schema the element schema, moved in and wrapped.
     */
    void set_items(SchemaObject schema) {
        m_items = std::make_shared<SchemaObject>(std::move(schema));
    }

    /**
     * @brief Grabs the JSON Schema type string.
     * @return the type, or empty if never set.
     */
    [[nodiscard]] const std::string &get_type() const noexcept { return m_type; }
    /**
     * @brief Grabs the format hint.
     * @return the format string, or empty if never set.
     */
    [[nodiscard]] const std::string &get_format() const noexcept { return m_format; }
    /**
     * @brief Checks the nullable flag.
     * @return true if null's allowed, false otherwise (the default — not nullable till told
     * otherwise).
     */
    [[nodiscard]] bool get_nullable() const noexcept { return m_nullable; }
    /**
     * @brief Grabs the $ref pointer.
     * @return the $ref string, or empty if this schema isn't a reference.
     */
    [[nodiscard]] const std::string &get_ref() const noexcept { return m_ref; }
    /**
     * @brief Grabs the required-property-name list.
     * @return every property name marked required, in insertion order — that's the whole W
     * right there, no reshuffling.
     */
    [[nodiscard]] const std::vector<std::string> &get_required() const noexcept {
        return m_required;
    }
    /**
     * @brief Grabs the allowed enum values.
     * @return every value added via add_enum_value(), in insertion order.
     */
    [[nodiscard]] const std::vector<std::string> &get_enum_values() const noexcept {
        return m_enum_values;
    }
    /**
     * @brief Grabs the full property map, no cap.
     * @return every named property schema, keyed by property name.
     */
    [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<SchemaObject>> &
    get_properties() const noexcept {
        return m_properties;
    }
    /**
     * @brief Grabs the array element schema.
     * @warning Only ever populated for array-typed schemas via set_items() — check get_type()
     * first, because an unset m_items comes back as a null shared_ptr and dereferencing that
     * blind is a straight UB L. Don't be that guy.
     * @return the element schema wrapped in a shared_ptr, null if this isn't an array schema.
     */
    [[nodiscard]] const std::shared_ptr<SchemaObject> &get_items() const noexcept { return m_items; }

  private:
    std::string m_type;
    std::string m_format;
    bool m_nullable{false};
    std::string m_ref;
    std::vector<std::string> m_required;
    std::vector<std::string> m_enum_values;
    std::unordered_map<std::string, std::shared_ptr<SchemaObject>> m_properties;
    std::shared_ptr<SchemaObject> m_items;
};

class Components {
  public:
    /**
     * @brief Default ctor — empty schema collection to start.
     */
    Components() = default;

    /**
     * @brief Registers a named schema under components.schemas.
     * @param name the schema's name.
     * @param schema the schema itself, moved in.
     */
    void add_schema(std::string name, SchemaObject schema) {
        m_schemas.emplace(std::move(name), std::move(schema));
    }

    /**
     * @brief Grabs every registered schema.
     * @return all named schemas, keyed by name.
     */
    [[nodiscard]] const std::unordered_map<std::string, SchemaObject> &get_schemas() const noexcept {
        return m_schemas;
    }

  private:
    std::unordered_map<std::string, SchemaObject> m_schemas;
};

class MediaType {
  public:
    /**
     * @brief Default ctor — blank schema until set_schema() gets called.
     */
    MediaType() = default;

    /**
     * @brief Sets the schema describing this media type's body shape.
     * @param schema the schema to store.
     */
    void set_schema(SchemaObject schema) { m_schema = std::move(schema); }

    /**
     * @brief Grabs the body schema, bet.
     * @return the schema for this media type.
     */
    [[nodiscard]] const SchemaObject &get_schema() const noexcept { return m_schema; }

  private:
    SchemaObject m_schema;
};

class RequestBody {
  public:
    /**
     * @brief Default ctor — required defaults to true, no content entries yet.
     */
    RequestBody() = default;

    /**
     * @brief Flags whether the request body is required.
     * @param value true if callers must send a body, false if it's optional.
     */
    void set_required(bool value) noexcept { m_required = value; }
    /**
     * @brief Registers a media-type entry (e.g. "application/json") for this body.
     * @param media_type the MIME type key.
     * @param media the media-type schema, moved in.
     */
    void add_content(std::string media_type, MediaType media) {
        m_content.emplace(std::move(media_type), std::move(media));
    }

    /**
     * @brief Checks whether the body's required, no cap.
     * @return true if required (the default — opt-out, not opt-in), false otherwise.
     */
    [[nodiscard]] bool get_required() const noexcept { return m_required; }
    /**
     * @brief Grabs the media-type map.
     * @return every registered media-type entry, keyed by MIME type.
     */
    [[nodiscard]] const std::unordered_map<std::string, MediaType> &get_content() const noexcept {
        return m_content;
    }

  private:
    bool m_required{true};
    std::unordered_map<std::string, MediaType> m_content;
};

class Response {
  public:
    /**
     * @brief Default ctor — description defaults to "OK", no content entries yet.
     */
    Response() = default;

    /**
     * @brief Sets the human-readable response description.
     * @param value the description text.
     */
    void set_description(std::string value) { m_description = std::move(value); }
    /**
     * @brief Registers a media-type entry (e.g. "application/json") for this response.
     * @param media_type the MIME type key.
     * @param media the media-type schema, moved in.
     */
    void add_content(std::string media_type, MediaType media) {
        m_content.emplace(std::move(media_type), std::move(media));
    }

    /**
     * @brief Grabs the response description.
     * @return the description, defaults to "OK" if never set — every response needs one per
     * the OpenAPI spec, so this class just ships a sane default instead of coming up empty.
     */
    [[nodiscard]] const std::string &get_description() const noexcept { return m_description; }
    /**
     * @brief Grabs the media-type map.
     * @return every registered media-type entry, keyed by MIME type.
     */
    [[nodiscard]] const std::unordered_map<std::string, MediaType> &get_content() const noexcept {
        return m_content;
    }

  private:
    std::string m_description{"OK"};
    std::unordered_map<std::string, MediaType> m_content;
};

class Operation {
  public:
    /**
     * @brief Default ctor — blank operation, no summary/tags/body/responses yet.
     */
    Operation() = default;

    /**
     * @brief Sets the one-liner summary shown in docs/UI. Keep it lowkey short, that's the
     * whole point of a summary.
     * @param value the summary text.
     */
    void set_summary(std::string value) { m_summary = std::move(value); }
    /**
     * @brief Sets the longer-form description for this operation.
     * @param value the description text.
     */
    void set_description(std::string value) { m_description = std::move(value); }
    /**
     * @brief Tags this operation for grouping (e.g. "tasks", "admin") — pure organizational
     * motion, no functional effect on the route itself.
     * @param tag the tag to add.
     */
    void add_tag(std::string tag) { m_tags.push_back(std::move(tag)); }
    /**
     * @brief Sets the request body spec, heap-wrapped since it's optional — most GETs won't
     * have one, so this stays null till a body actually gets attached.
     * @param body the request body, moved in and wrapped.
     */
    void set_request_body(RequestBody body) {
        m_request_body = std::make_shared<RequestBody>(std::move(body));
    }
    /**
     * @brief Registers a response for a given status code.
     * @param status the status code as a string (OpenAPI keys responses by string, not int).
     * @param response the response spec, moved in.
     */
    void add_response(std::string status, Response response) {
        m_responses.emplace(std::move(status), std::move(response));
    }

    /**
     * @brief Grabs the summary text.
     * @return the summary, or empty if never set.
     */
    [[nodiscard]] const std::string &get_summary() const noexcept { return m_summary; }
    /**
     * @brief Grabs the description text.
     * @return the description, or empty if never set.
     */
    [[nodiscard]] const std::string &get_description() const noexcept { return m_description; }
    /**
     * @brief Grabs the tag list.
     * @return every tag added via add_tag(), in insertion order.
     */
    [[nodiscard]] const std::vector<std::string> &get_tags() const noexcept { return m_tags; }
    /**
     * @brief Grabs the request body spec.
     * @warning Comes back null if set_request_body() was never called — most GET operations won't
     * have a body, so check before dereferencing, don't just yolo it.
     * @return the request body wrapped in a shared_ptr, null if none was set. Handle the null
     * case or it's an L waiting to happen.
     */
    [[nodiscard]] const std::shared_ptr<RequestBody> &get_request_body() const noexcept {
        return m_request_body;
    }
    /**
     * @brief Grabs the response map.
     * @return every registered response, keyed by status code string.
     */
    [[nodiscard]] const std::unordered_map<std::string, Response> &get_responses() const noexcept {
        return m_responses;
    }

  private:
    std::string m_summary;
    std::string m_description;
    std::vector<std::string> m_tags;
    std::shared_ptr<RequestBody> m_request_body;
    std::unordered_map<std::string, Response> m_responses;
};

class Info {
  public:
    /**
     * @brief Default ctor — title/version start at their defaults ("Congelado API" / "1.0.0").
     */
    Info() = default;

    /**
     * @brief Sets the API title.
     * @param value the title text.
     */
    void set_title(std::string value) { m_title = std::move(value); }
    /**
     * @brief Sets the API version string.
     * @param value the version text.
     */
    void set_version(std::string value) { m_version = std::move(value); }

    /**
     * @brief Grabs the API title, bet.
     * @return the title, defaults to "Congelado API".
     */
    [[nodiscard]] const std::string &get_title() const noexcept { return m_title; }
    /**
     * @brief Grabs the API version string.
     * @return the version, defaults to "1.0.0".
     */
    [[nodiscard]] const std::string &get_version() const noexcept { return m_version; }

  private:
    std::string m_title{"Congelado API"};
    std::string m_version{"1.0.0"};
};

// Document.paths maps "/full/path" -> ("get"/"post"/... -> Operation) directly, with no
// wrapper class in between, so the JSON shape is exactly {"paths": {"/x": {"get": {...}}}}
// as required by the OpenAPI spec (a wrapper class here would add an unwanted extra key).
class Document {
  public:
    /**
     * @brief Default ctor — openapi version defaults to "3.0.3", everything else starts blank.
     */
    Document() = default;

    /**
     * @brief Sets the OpenAPI spec version string. Not this class's own version, don't get it
     * twisted with Info::set_version() — this one's the spec version itself.
     * @param value the version string, e.g. "3.0.3".
     */
    void set_openapi(std::string value) { m_openapi = std::move(value); }
    /**
     * @brief Sets the document's info block (title/version).
     * @param info the info block, moved in.
     */
    void set_info(Info info) { m_info = std::move(info); }
    /**
     * @brief Sets the document's components block (named schemas).
     * @param components the components block, moved in.
     */
    void set_components(Components components) { m_components = std::move(components); }
    /**
     * @brief Registers an operation under a path + method — the actual "paths" map build step,
     * wired directly into the nested-map shape the OpenAPI spec expects (see the note above
     * this class for why there's no wrapper class in between).
     * @param path the full request path (e.g. "/tasks/{id}").
     * @param method the HTTP method, lowercased (e.g. "get", "post").
     * @param operation the operation spec, moved in.
     */
    void add_operation(std::string path, std::string method, Operation operation) {
        m_paths[std::move(path)][std::move(method)] = std::move(operation);
    }

    /**
     * @brief Grabs the OpenAPI spec version string.
     * @return the version string, defaults to "3.0.3".
     */
    [[nodiscard]] const std::string &get_openapi() const noexcept { return m_openapi; }
    /**
     * @brief Grabs the info block.
     * @return the document's info block.
     */
    [[nodiscard]] const Info &get_info() const noexcept { return m_info; }
    /**
     * @brief Grabs the components block.
     * @return the document's components block.
     */
    [[nodiscard]] const Components &get_components() const noexcept { return m_components; }
    /**
     * @brief Grabs the full paths map — the whole route table, straight up.
     * @return every registered operation, keyed first by path then by lowercased method.
     */
    [[nodiscard]] const std::unordered_map<std::string, std::unordered_map<std::string, Operation>> &
    get_paths() const noexcept {
        return m_paths;
    }

  private:
    std::string m_openapi{"3.0.3"};
    Info m_info;
    Components m_components;
    std::unordered_map<std::string, std::unordered_map<std::string, Operation>> m_paths;
};

} // namespace utils::openapi

template <>
struct serde::Serializable<utils::openapi::SchemaObject> {
    /**
     * @brief Field-descriptor table wiring SchemaObject's JSON keys ("type", "$ref", "items",
     * etc.) to its getters/setters — this is what serde actually walks to (de)serialize the
     * type, no reflection macros or manual JSON code needed anywhere else.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        using utils::openapi::SchemaObject;
        return std::tuple{
            serde::FieldDesc<"type", &SchemaObject::get_type, &SchemaObject::set_type>{},
            serde::FieldDesc<"format", &SchemaObject::get_format, &SchemaObject::set_format>{},
            serde::FieldDesc<"$ref", &SchemaObject::get_ref, &SchemaObject::set_ref>{},
            serde::FieldDesc<"nullable", &SchemaObject::get_nullable,
                             &SchemaObject::set_nullable>{},
            serde::FieldDesc<"required", &SchemaObject::get_required,
                             &SchemaObject::add_required>{},
            serde::FieldDesc<"enum", &SchemaObject::get_enum_values,
                             &SchemaObject::add_enum_value>{},
            serde::FieldDesc<"properties", &SchemaObject::get_properties,
                             &SchemaObject::add_property>{},
            serde::FieldDesc<"items", &SchemaObject::get_items, &SchemaObject::set_items>{},
        };
    }
};

template <>
struct serde::Serializable<utils::openapi::Components> {
    /**
     * @brief Field-descriptor table wiring Components' "schemas" key to get_schemas()/add_schema().
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        using utils::openapi::Components;
        return std::tuple{
            serde::FieldDesc<"schemas", &Components::get_schemas, &Components::add_schema>{},
        };
    }
};

template <>
struct serde::Serializable<utils::openapi::MediaType> {
    /**
     * @brief Field-descriptor table wiring MediaType's "schema" key to get_schema()/set_schema().
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        using utils::openapi::MediaType;
        return std::tuple{
            serde::FieldDesc<"schema", &MediaType::get_schema, &MediaType::set_schema>{},
        };
    }
};

template <>
struct serde::Serializable<utils::openapi::RequestBody> {
    /**
     * @brief Field-descriptor table wiring RequestBody's "required"/"content" keys to their
     * getters/setters.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        using utils::openapi::RequestBody;
        return std::tuple{
            serde::FieldDesc<"required", &RequestBody::get_required,
                             &RequestBody::set_required>{},
            serde::FieldDesc<"content", &RequestBody::get_content, &RequestBody::add_content>{},
        };
    }
};

template <>
struct serde::Serializable<utils::openapi::Response> {
    /**
     * @brief Field-descriptor table wiring Response's "description"/"content" keys to their
     * getters/setters.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        using utils::openapi::Response;
        return std::tuple{
            serde::FieldDesc<"description", &Response::get_description,
                             &Response::set_description>{},
            serde::FieldDesc<"content", &Response::get_content, &Response::add_content>{},
        };
    }
};

template <>
struct serde::Serializable<utils::openapi::Operation> {
    /**
     * @brief Field-descriptor table wiring Operation's keys ("summary", "tags", "requestBody",
     * "responses", etc.) to their getters/setters — this is the whole reason .body<T>()/
     * .response<T>() can round-trip through serde::Json::encode() without any bespoke encoder.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        using utils::openapi::Operation;
        return std::tuple{
            serde::FieldDesc<"summary", &Operation::get_summary, &Operation::set_summary>{},
            serde::FieldDesc<"description", &Operation::get_description,
                             &Operation::set_description>{},
            serde::FieldDesc<"tags", &Operation::get_tags, &Operation::add_tag>{},
            serde::FieldDesc<"requestBody", &Operation::get_request_body,
                             &Operation::set_request_body>{},
            serde::FieldDesc<"responses", &Operation::get_responses,
                             &Operation::add_response>{},
        };
    }
};

template <>
struct serde::Serializable<utils::openapi::Info> {
    /**
     * @brief Field-descriptor table wiring Info's "title"/"version" keys to their
     * getters/setters. Small class, small table, no cap.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        using utils::openapi::Info;
        return std::tuple{
            serde::FieldDesc<"title", &Info::get_title, &Info::set_title>{},
            serde::FieldDesc<"version", &Info::get_version, &Info::set_version>{},
        };
    }
};

template <>
struct serde::Serializable<utils::openapi::Document> {
    /**
     * @brief Field-descriptor table wiring Document's top-level keys ("openapi", "info",
     * "components", "paths") to their getters/setters — the root of the whole serialized
     * document, this is what serde::Json::encode(document) ultimately walks. Whole W right
     * here, the entire spec output funnels through this one table.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        using utils::openapi::Document;
        return std::tuple{
            serde::FieldDesc<"openapi", &Document::get_openapi, &Document::set_openapi>{},
            serde::FieldDesc<"info", &Document::get_info, &Document::set_info>{},
            serde::FieldDesc<"components", &Document::get_components, &Document::set_components>{},
            serde::FieldDesc<"paths", &Document::get_paths, &Document::add_operation>{},
        };
    }
};
