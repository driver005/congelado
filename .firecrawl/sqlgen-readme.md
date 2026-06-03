\# !\[C++\](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) sqlgen

\[!\[License: MIT\](https://img.shields.io/badge/License-MIT-yellow.svg)\](https://opensource.org/licenses/MIT)
\[!\[Maintenance\](https://img.shields.io/badge/Maintained%3F-yes-green.svg)\](https://github.com/getml/reflect-cpp/graphs/commit-activity)
\[!\[Generic badge\](https://img.shields.io/badge/C++-20-blue.svg)\](https://shields.io/)
\[!\[Generic badge\](https://img.shields.io/badge/gcc-11+-blue.svg)\](https://shields.io/)
\[!\[Generic badge\](https://img.shields.io/badge/clang-14+-blue.svg)\](https://shields.io/)
\[!\[Generic badge\](https://img.shields.io/badge/MSVC-17+-blue.svg)\](https://shields.io/)
\[!\[Conan Center\](https://img.shields.io/conan/v/sqlgen)\](https://conan.io/center/recipes/sqlgen)

\*\*📖 Documentation\*\*: \[Click here\](docs/README.md)

\*\*sqlgen\*\* is a modern, type-safe ORM and SQL query generator for C++20, inspired by Python's \[SQLAlchemy\](https://github.com/sqlalchemy/sqlalchemy)/\[SQLModel\](https://github.com/fastapi/sqlmodel) and Rust's \[Diesel\](https://github.com/diesel-rs/diesel). It provides a fluent, composable interface for database operations with compile-time type checking and SQL injection protection.

sqlgen is based on and tightly integrated with \[reflect-cpp\](https://github.com/getml/reflect-cpp), a C++-20 library for fast serialization, deserialization and validation using reflection, similar to pydantic in Python, serde in Rust, encoding in Go or aeson in Haskell.

Together, reflect-cpp and sqlgen enable reliable and efficient ETL pipelines.

\## Features

\- 🔒 \*\*Type Safety\*\*: Compile-time validation of table schemas and queries
\- 🛡️ \*\*SQL Injection Protection\*\*: Built-in input validation and parameterized queries
\- 🔄 \*\*Composable Queries\*\*: Fluent interface for building complex queries
\- 🚀 \*\*High Performance\*\*: Efficient batch operations and prepared statements
\- 📦 \*\*Modern C++\*\*: Leverages C++20 features for a clean, expressive API
\- 🔌 \*\*Multiple Backends\*\*: Support for PostgreSQL and SQLite
\- 🔍 \*\*Reflection Integration\*\*: Seamless integration with \[reflect-cpp\](https://github.com/getml/reflect-cpp)

\## Supported databases

The following table lists the databases currently supported by sqlgen and the underlying libraries used:

\| Database \| Library \| Version \| License \| Remarks \|
\|---------------\|--------------------------------------------------------------------------\|--------------\|---------------\| -----------------------------------------------------\|
\| DuckDB \| \[duckdb\](https://github.com/duckdb/duckdb) \| >= 1.4.2 \| MIT \| \|
\| MySQL/MariaDB \| \[libmariadb\](https://github.com/mariadb-corporation/mariadb-connector-c) \| >= 3.4.5 \| LGPL \| \|
\| PostgreSQL \| \[libpq\](https://github.com/postgres/postgres) \| >= 16.4 \| PostgreSQL \| Will work for all libpq-compatible databases \|
\| sqlite \| \[sqlite\](https://sqlite.org/index.html) \| >= 3.49.1 \| Public Domain \| \|

\## Quick Start

\### Install using vcpkg or Conan

You can install the latest release of sqlgen
using either \[vcpkg\](https://vcpkg.io/en/package/sqlgen) or \[Conan\](https://conan.io/center/recipes/sqlgen).

\### Build using vcpkg

Alternatively, you can build sqlgen from source using vcpkg:

1\. Make sure you have the required dependencies installed (skip this step on Windows):
\`\`\`bash
sudo apt-get install autoconf bison flex # Linux (Ubuntu, Debian, ...)
brew install autoconf bison flex # macOS
\`\`\`

2\. Set up vcpkg:
\`\`\`bash
git submodule update --init
./vcpkg/bootstrap-vcpkg.sh # Linux, macOS
./vcpkg/bootstrap-vcpkg.bat # Windows
\`\`\`

3\. Build the library:
\`\`\`bash
cmake -S . -B build -DCMAKE\_CXX\_STANDARD=20 -DCMAKE\_BUILD\_TYPE=Release
cmake --build build -j 4 # gcc, clang
cmake --build build --config Release -j 4 # MSVC
\`\`\`

This will build the static library. To build the shared library
add \`-DBUILD\_SHARED\_LIBS=ON -DVCPKG\_TARGET\_TRIPLET=...\` to the first line.
Run \`./vcpkg/vcpkg help triplets\` to view all supported triplets.
Common triplets for shared libraries are \`x64-linux-dynamic\`,
\`arm64-osx-dynamic\` or \`x64-osx-dynamic\`.

Add \`-DSQLGEN\_MYSQL=ON\` to support MySQL/MariaDB. Add \`-DSQLGEN\_DUCKDB=ON\` to support DuckDB.

4\. Include in your CMake project:
\`\`\`cmake
find\_package(sqlgen REQUIRED)
target\_link\_libraries(your\_target PRIVATE sqlgen::sqlgen)
\`\`\`

\### Build using Conan

You can also build sqlgen from source using Conan:

1\. Install Conan (assuming you have Python and pipx installed):

\`\`\`bash
pipx install conan
conan profile detect
\`\`\`

For older versions of pip, you can also use \`pip\` instead of \`pipx\`.

2\. Build the library:

\`\`\`bash
conan build . --build=missing -s compiler.cppstd=gnu20
\`\`\`

This will build the static library. To build the shared library,
add \`-o \*/\*:shared=True\`.

Add \`-o sqlgen/\*:with\_mysql=True\` to support MySQL/MariaDB.

3\. Include in your CMake project:
\`\`\`cmake
find\_package(sqlgen REQUIRED)
target\_link\_libraries(your\_target PRIVATE sqlgen::sqlgen)
\`\`\`

You can call \`conan inspect .\` to get an overview of the supported options.

\## Usage Examples

\### Hello World

\`\`\`cpp
#include
#include

struct User {
 std::string name;
 int age;
};

int main() {
 // Connect to SQLite database
 const auto conn = sqlgen::sqlite::connect("test.db");

 // Create and insert a user
 const auto user = User{.name = "John", .age = 30};
 sqlgen::write(conn, user);

 // Read all users
 const auto users = sqlgen::read>(conn).value();

 for (const auto& u : users) {
 std::cout << u.name << " is " << u.age << " years old\\n";
 }
}
\`\`\`

\### Connecting to a Database

\`\`\`cpp
#include

// PostgreSQL connection
const auto credentials = sqlgen::postgres::Credentials{
 .user = "username",
 .password = "password",
 .host = "localhost",
 .dbname = "mydb",
 .port = 5432
};

const auto conn = sqlgen::postgres::connect(credentials);

// SQLite connection
const auto sqlite\_conn = sqlgen::sqlite::connect("database.db");
\`\`\`

\### Defining Models

\`\`\`cpp
struct Person {
 std::string first\_name;
 std::string last\_name;
 uint32\_t age;
 std::optional email; // Nullable field
};
\`\`\`

\### Inserting Data

\`\`\`cpp
const auto people = std::vector({
 Person{.first\_name = "Homer", .last\_name = "Simpson", .age = 45},
 Person{.first\_name = "Marge", .last\_name = "Simpson", .age = 42}
});

// Automatically creates table if it doesn't exist
// (recall that the table is fully defined at compile time)
const auto result = sqlgen::write(conn, people);

if (!result) {
 std::cerr << "Error: " << result.error().what() << std::endl;
}
\`\`\`

Or:

\`\`\`cpp
...

// write(...) abstracts these steps away, but
// if you prefer more granular control, you
// can use sqlgen::insert.
const auto result = begin\_transaction(conn)
 .and\_then(create\_table \| if\_not\_exists)
 .and\_then(insert(std::ref(people)))
 .and\_then(commit);

...
\`\`\`

Generated SQL:
\`\`\`sql
BEGIN TRANSACTION;
CREATE TABLE IF NOT EXISTS "Person" (
 "first\_name" TEXT NOT NULL,
 "last\_name" TEXT NOT NULL,
 "age" INTEGER NOT NULL,
 "email" TEXT
);

INSERT INTO "Person" ("first\_name", "last\_name", "age", "email")
VALUES (?, ?, ?, ?);
COMMIT;
\`\`\`

\### Querying Data

\`\`\`cpp
#include
#include

using namespace sqlgen;
using namespace sqlgen::literals;

// Build a query for adults, ordered by age
const auto query = read\> \|
 where("age"\_c >= 18) \|
 order\_by("age"\_c.desc(), "last\_name"\_c) \|
 limit(10);

// Execute the query
const auto result = query(conn);

if (result) {
 // Print results as JSON
 std::cout << rfl::json::write(\*result) << std::endl;
} else {
 std::cerr << "Error: " << result.error().what() << std::endl;
}
\`\`\`

Generated SQL:
\`\`\`sql
SELECT "first\_name", "last\_name", "age", "email"
FROM "Person"
WHERE "age" >= 18
ORDER BY "age" DESC, "last\_name"
LIMIT 10;
\`\`\`

\### Grouping and Aggregating Data

\`\`\`cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct Children {
 std::string last\_name;
 int num\_children;
 int max\_age;
 int min\_age;
 int sum\_age;
};

const auto get\_children = select\_from(
 "last\_name"\_c,
 count().as<"num\_children">(),
 max("age"\_c).as<"max\_age">(),
 min("age"\_c).as<"min\_age">(),
 sum("age"\_c).as<"sum\_age">(),
) \| where("age"\_c < 18) \| group\_by("last\_name"\_c) \| to>;

const std::vector children = get\_children(conn).value();
\`\`\`

Generated SQL:
\`\`\`sql
SELECT
 "last\_name",
 COUNT(\*) as "num\_children",
 MAX("age") as "max\_age",
 MIN("age") as "min\_age",
 SUM("age") as "sum\_age"
FROM "Person"
WHERE "age" < 18
GROUP BY "last\_name";
\`\`\`

\### Joining data

\`\`\`cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct ParentAndChild {
 std::string last\_name;
 std::string first\_name\_parent;
 std::string first\_name\_child;
 double parent\_age\_at\_birth;
};

const auto get\_people =
 select\_from(
 "last\_name"\_t1 \| as<"last\_name">,
 "first\_name"\_t1 \| as<"first\_name\_parent">,
 "first\_name"\_t3 \| as<"first\_name\_child">,
 ("age"\_t1 - "age"\_t3) \| as<"parent\_age\_at\_birth">) \|
 inner\_join("id"\_t1 == "parent\_id"\_t2) \|
 inner\_join("id"\_t3 == "child\_id"\_t2) \|
 order\_by("id"\_t1, "id"\_t3) \| to>;
\`\`\`

Generated SQL:
\`\`\`sql
SELECT t1."last\_name" AS "last\_name",
 t1."first\_name" AS "first\_name\_parent",
 t3."first\_name" AS "first\_name\_child",
 t1."age" - t3."age" AS "parent\_age\_at\_birth"
FROM "Person" t1
INNER JOIN "Relationship" t2
ON t1."id" = t2."parent\_id"
INNER JOIN "Person" t3
ON t3."id" = t2."child\_id"
ORDER BY t1."id", t3."id"
\`\`\`

\### Nested joins

\`\`\`cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct ParentAndChild {
 std::string last\_name;
 std::string first\_name\_parent;
 std::string first\_name\_child;
 double parent\_age\_at\_birth;
};

// First, create a subquery
const auto get\_children =
 select\_from("parent\_id"\_t1 \| as<"id">,
 "first\_name"\_t2 \| as<"first\_name">,
 "age"\_t2 \| as<"age">) \|
 inner\_join("id"\_t2 == "child\_id"\_t1);

// Then use it as a source for another query
const auto get\_people =
 select\_from(
 "last\_name"\_t1 \| as<"last\_name">,
 "first\_name"\_t1 \| as<"first\_name\_parent">,
 "first\_name"\_t2 \| as<"first\_name\_child">,
 ("age"\_t1 - "age"\_t2) \| as<"parent\_age\_at\_birth">) \|
 inner\_join<"t2">(
 get\_children, // Use the subquery as the source
 "id"\_t1 == "id"\_t2) \|
 order\_by("id"\_t1, "id"\_t2) \| to>;
\`\`\`

Generated SQL:
\`\`\`sql
SELECT t1."last\_name" AS "last\_name",
 t1."first\_name" AS "first\_name\_parent",
 t2."first\_name" AS "first\_name\_child",
 t1."age" - t2."age" AS "parent\_age\_at\_birth"
FROM "Person" t1
INNER JOIN (
 SELECT t1."parent\_id" AS "id",
 t2."first\_name" AS "first\_name",
 t2."age" AS "age"
 FROM "Relationship" t1
 INNER JOIN "Person" t2
 ON t2."id" = t1."child\_id"
) t2
ON t1."id" = t2."id"
ORDER BY t1."id", t2."id"
\`\`\`

Or:
\`\`\`cpp
using namespace sqlgen;
using namespace sqlgen::literals;

struct ParentAndChild {
 std::string last\_name;
 std::string first\_name\_parent;
 std::string first\_name\_child;
 double parent\_age\_at\_birth;
};

// First, create a subquery
const auto get\_parents = select\_from(
 "child\_id"\_t2 \| as<"id">,
 "first\_name"\_t1 \| as<"first\_name">,
 "last\_name"\_t1 \| as<"last\_name">,
 "age"\_t1 \| as<"age">
) \| inner\_join("id"\_t1 == "parent\_id"\_t2);

// Then use it as a source for another query
const auto get\_people = select\_from<"t1">(
 get\_parents, // Use the subquery as the source
 "last\_name"\_t1 \| as<"last\_name">,
 "first\_name"\_t1 \| as<"first\_name\_parent">,
 "first\_name"\_t2 \| as<"first\_name\_child">,
 ("age"\_t1 - "age"\_t2) \| as<"parent\_age\_at\_birth">) \|
 inner\_join("id"\_t1 == "id"\_t2) \|
 order\_by("id"\_t1, "id"\_t2) \| to>;
\`\`\`

Generated SQL:
\`\`\`sql
SELECT t1."last\_name" AS "last\_name",
 t1."first\_name" AS "first\_name\_parent",
 t2."first\_name" AS "first\_name\_child",
 (t1."age") - (t2."age") AS "parent\_age\_at\_birth"
FROM (
 SELECT t2."child\_id" AS "id",
 t1."first\_name" AS "first\_name",
 t1."last\_name" AS "last\_name",
 t1."age" AS "age"
 FROM "Person" t1
 INNER JOIN "Relationship" t2
 ON t1."id" = t2."parent\_id"
) t1
INNER JOIN "Person" t2
ON t1."id" = t2."id"
ORDER BY t1."id", t2."id"
\`\`\`

\### Type Safety and SQL Injection Protection

sqlgen provides comprehensive compile-time checks and runtime protection:

\`\`\`cpp
// Compile-time error: No such column "color"
const auto query = read\> \|
 where("color"\_c == "blue");

// Compile-time error: Cannot compare column "age" to a string
const auto query = read\> \|
 where("age"\_c == "Homer");

// Compile-time error: "age" must be aggregated or included in GROUP BY
const auto query = select\_from(
 "last\_name"\_c,
 "age"\_c
) \| group\_by("last\_name"\_c);

// Compile-time error: Cannot add string and int
const auto query = select\_from(
 "last\_name"\_c + "age"\_c
);

// Runtime protection against SQL injection
std::vector get\_people(const auto& conn,
 const sqlgen::AlphaNumeric& first\_name) {
 using namespace sqlgen;
 return (read\> \|
 where("first\_name"\_c == first\_name))(conn).value();
}

// This will be rejected
get\_people(conn, "Homer' OR '1'='1"); // SQL injection attempt
\`\`\`

\## Documentation

For detailed documentation, visit our \[documentation page\](docs/README.md).

\## Contributing

We welcome constructive criticism, feature requests and contributions! Please open an issue or a pull request.

\## License

This project is licensed under the MIT License - see the \[LICENSE\](LICENSE) file for details.