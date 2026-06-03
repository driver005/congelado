\# \`sqlgen::JSON\`

\`sqlgen::JSON\` stores structured JSON data in your database while preserving full C++ type-safety. It integrates with reflectcpp (\`rfl\`) so you can read and write arbitrary structured data without manual (de)serialization.

Critically, \`rfl::JSON\` is fully serializable with reflectcpp and supports any data type also supported by reflectcpp (including nested structs, \`std::optional\`, containers like \`std::vector\`, and more), enabling seamless end-to-end integration.

\## Usage

\### Basic Definition

Define a JSON field in your struct by wrapping the underlying C++ type with \`sqlgen::JSON\`:

\`\`\`cpp
struct Person {
 std::string first\_name;
 std::string last\_name;
 int age;
 sqlgen::JSON>\> children; // Nested, optional JSON
};
\`\`\`

This generates a table with a JSON-compatible column type (dialect-specific):

\`\`\`sql
\-\- PostgreSQL
CREATE TABLE IF NOT EXISTS "Person"(
 "first\_name" TEXT NOT NULL,
 "last\_name" TEXT NOT NULL,
 "age" INTEGER NOT NULL,
 "children" JSONB
);

\-\- MySQL / MariaDB
CREATE TABLE IF NOT EXISTS \`Person\`(
 \`first\_name\` TEXT NOT NULL,
 \`last\_name\` TEXT NOT NULL,
 \`age\` INT NOT NULL,
 \`children\` JSON
);

\-\- SQLite
CREATE TABLE IF NOT EXISTS "Person"(
 "first\_name" TEXT NOT NULL,
 "last\_name" TEXT NOT NULL,
 "age" INTEGER NOT NULL,
 "children" JSONB
);
\`\`\`

\### Construction and Assignment

Assign any reflectcpp-supported value to the JSON field. For example, a nested vector of the same type:

\`\`\`cpp
const auto children = std::vector({
 Person{.first\_name = "Bart", .last\_name = "Simpson", .age = 10},
 Person{.first\_name = "Lisa", .last\_name = "Simpson", .age = 8},
 Person{.first\_name = "Maggie", .last\_name = "Simpson", .age = 0}
});

const auto homer = Person{
 .first\_name = "Homer",
 .last\_name = "Simpson",
 .age = 45,
 .children = children // Automatically serialized to JSON
};
\`\`\`

You can store any \`T\` that reflectcpp can serialize, such as:
\- \`std::optional\`
\- \`std::vector\`, \`std::map\`, and other standard containers
\- Nested \`struct\` types reflected with reflectcpp

\### Reading and Writing

Use the regular \`sqlgen::write\` and \`sqlgen::read\` APIs. JSON values are transparently serialized/deserialized.

\`\`\`cpp
using namespace sqlgen;
using namespace sqlgen::literals;

// PostgreSQL example (analogous for MySQL/SQLite)
const auto credentials = sqlgen::postgres::Credentials{
 .user = "postgres", .password = "password", .host = "localhost", .dbname = "postgres"};

const auto people = sqlgen::postgres::connect(credentials)
 .and\_then(drop \| if\_exists)
 .and\_then(write(std::ref(homer)))
 .and\_then(sqlgen::read>)
 .value();
\`\`\`

\### Accessing Values

Access the underlying C++ value via familiar methods:

\`\`\`cpp
// Underlying value access
people\[0\].children();
people\[0\].children.get();
people\[0\].children.value();

// Serialize entire result to JSON with reflectcpp
const std::string json = rfl::json::write(people);
\`\`\`

\### Template Parameters

The \`sqlgen::JSON\` template takes one parameter:

1\. T: Any reflectcpp-supported C++ type to store as JSON

\`\`\`cpp
sqlgen::JSON field\_name;
\`\`\`

\### Database Integration

\`sqlgen::JSON\` selects the appropriate JSON-capable storage per dialect:
\- \*\*PostgreSQL\*\*: \`JSONB\`
\- \*\*MySQL/MariaDB\*\*: \`JSON\`
\- \*\*SQLite\*\*: \`JSONB\`

All (de)serialization is handled by reflectcpp (\`rfl\`) underneath, ensuring type-safe transformations without manual glue code.

\## Notes

\- Works with any reflectcpp-serializable type (\`rfl::JSON\` support), including deeply nested structures
\- Integrates with \`sqlgen::read\` and \`sqlgen::write\` like any other field
\- Database JSON type is chosen automatically per dialect
\- Supports move and copy semantics
\- Provides multiple access methods for the underlying value
\- Ideal for flexible, schema-evolving nested attributes