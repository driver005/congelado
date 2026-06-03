\# \`sqlgen::read\`

The \`sqlgen::read\` interface provides a flexible and type-safe way to query data from a SQL database into C++ containers or ranges. It supports composable query building with \`where\`, \`order\_by\`, and \`limit\` clauses.

\## Usage

\### Basic Read

Read all rows from a table into a container (e.g., \`std::vector\`):

\`\`\`cpp
const auto conn = sqlgen::sqlite::connect("database.db");

const std::vector people = sqlgen::read>(conn).value();
\`\`\`

This generates the following SQL:

\`\`\`sql
SELECT "id", "first\_name", "last\_name", "age"
FROM "Person";
\`\`\`

Note that \`conn\` is actually a connection wrapped into an \`sqlgen::Result<...>\`.
This means you can use monadic error handling and fit this into a single line:

\`\`\`cpp
// sqlgen::Result>
const auto people = sqlgen::sqlite::connect("database.db").and\_then(
 sqlgen::read>);
\`\`\`

Please refer to the documentation on \`sqlgen::Result<...>\` for more information on error handling.

\### With \`where\` clause

Filter results using a \`where\` clause:

\`\`\`cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = sqlgen::read\> \|
 where("age"\_c < 18 and "first\_name"\_c != "Hugo");

const auto minors = query(conn).value();
\`\`\`

This generates the following SQL:

\`\`\`sql
SELECT "id", "first\_name", "last\_name", "age"
FROM "Person"
WHERE
 ("age" < 18) AND
 ("first\_name" != 'Hugo');
\`\`\`

Note that \`"..."\_c\` refers to the name of the column. If such a field does not
exists on the struct \`Person\`, the code will fail to compile. It is defined in
the namespace \`sqlgen::literals\`.

You can also use monadic error handling here:

\`\`\`cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = sqlgen::read\> \|
 where("age"\_c < 18 and "first\_name"\_c != "Hugo");

// sqlgen::Result>
const auto minors = sqlite::connect("database.db").and\_then(query);
\`\`\`

\### With \`order\_by\` and \`limit\`

Sort and limit results:

\`\`\`cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = sqlgen::read\> \|
 order\_by("age"\_c) \|
 limit(2);

const auto youngest\_two = query(conn).value();
\`\`\`

This generates the following SQL:

\`\`\`sql
SELECT "id", "first\_name", "last\_name", "age"
FROM "Person"
ORDER BY "age"
LIMIT 2;
\`\`\`

You can also combine \`limit\` with \`offset\` to perform paging:

\`\`\`cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = sqlgen::read\> \|
 order\_by("age"\_c) \|
 limit(2) \|
 offset(3);

const auto skip\_three = query(conn).value();
\`\`\`

This generates the following SQL:

\`\`\`sql
SELECT "id", "first\_name", "last\_name", "age"
FROM "Person"
ORDER BY "age"
LIMIT 2
OFFSET 3;
\`\`\`

\- \*\*SQLite and MySql Limitation\*: You cannot use \`offset\` without \`limit\`.

\### With ranges

Read results as a lazy range:

\`\`\`cpp
const auto people\_range = sqlgen::read>(conn).value();

for (const sqlgen::Result& person : people\_range) {
 // process result
}
\`\`\`

\`people\_range\` satisfies the \`std::ranges::input\_range\` concept, making it compatible with C++20 ranges and views. This allows for memory-efficient iteration through database results and enables composition with other range operations:

\`\`\`cpp
using namespace std::ranges::views;

// Transform range results
const auto first\_names = people\_range \| transform(\[\](const sqlgen::Result& r) {
 return r.value().first\_name;
});

// Filter range results
const auto adults = people\_range \| filter(\[\](const sqlgen::Result& r) {
 return r && r->age >= 18;
});
\`\`\`

\## Example: Full Query Composition

\`\`\`cpp
using namespace sqlgen;
using namespace sqlgen::literals;

const auto query = sqlgen::read\> \|
 where("age"\_c >= 18) \|
 order\_by("last\_name"\_c, "first\_name"\_c.desc()) \|
 limit(10);

const auto adults = query(conn).value();
\`\`\`

This generates the following SQL:

\`\`\`sql
SELECT "id", "first\_name", "last\_name", "age"
FROM "Person"
WHERE
 ("age" >= 18)
ORDER BY
 "last\_name",
 "first\_name" DESC
LIMIT 10;
\`\`\`

It is strongly recommended that you use \`using namespace sqlgen\` and \`using namespace sqlgen::literals;\`. However,
if you do not want to do that, you can rewrite the example above as follows:

\`\`\`cpp
const auto query = sqlgen::read\> \|
 sqlgen::where(sqlgen::col<"age"> >= 18) \|
 sqlgen::order\_by(sqlgen::col<"last\_name">, sqlgen::col<"first\_name">.desc()) \|
 sqlgen::limit(10);

const auto adults = query(conn).value();
\`\`\`

\## Notes

\- All query clauses (\`where\`, \`order\_by\`, \`limit\`) are optional.
\- The \`Result\` type provides error handling; use \`.value()\` to extract the result (will throw a exception if the results) or handle errors as needed. Refer to the
\- The \`sqlgen::Range\` type allows for lazy iteration over results.
\- \`"..."\_c\` refers to the name of the column.