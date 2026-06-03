\# \`sqlgen::PrimaryKey\`

\`sqlgen::PrimaryKey\` is used to indicate which key should be a primary key.

\## Usage

\### Basic Definition

Define a primary key field in your struct:

\`\`\`cpp
struct People {
 sqlgen::PrimaryKey first\_name;
 std::string last\_name;
 uint age;
};
\`\`\`

This generates the following SQL schema:

\`\`\`sql
CREATE TABLE IF NOT EXISTS "People"(
 "first\_name" TEXT NOT NULL,
 "last\_name" TEXT NOT NULL,
 "age" INTEGER NOT NULL,
 PRIMARY\_KEY("first\_name")
);
\`\`\`

\### Multiple Primary Keys

You can define multiple primary keys by using \`sqlgen::PrimaryKey\` on multiple fields. This will create a composite primary key.

\`\`\`cpp
struct Order {
 sqlgen::PrimaryKey order\_id;
 sqlgen::PrimaryKey product\_id;
 int quantity;
};
\`\`\`

Now the generated SQL schema will look like this:

\`\`\`sql
CREATE TABLE IF NOT EXISTS "Order"(
 "order\_id" INTEGER NOT NULL,
 "product\_id" INTEGER NOT NULL,
 "quantity" INTEGER NOT NULL,
 PRIMARY KEY("order\_id", "product\_id")
);
\`\`\`

Note that this is not supported in SQLite, as it does not support composite primary keys.

\### Auto-incrementing Primary Keys

You can define an auto-incrementing primary key by providing \`sqlgen::auto\_incr\` as the second template argument to \`sqlgen::PrimaryKey\`. The underlying type of an auto-incrementing primary key must be an integral type.

\`\`\`cpp
struct Person {
 sqlgen::PrimaryKey id;
 std::string first\_name;
 std::string last\_name;
 int age;
};
\`\`\`

This will produce SQL schema with an auto-incrementing primary key. For instance, for PostgreSQL it will generate:

\`\`\`sql
CREATE TABLE IF NOT EXISTS "Person"(
 "id" INTEGER GENERATED ALWAYS AS IDENTITY,
 "first\_name" TEXT NOT NULL,
 "last\_name" TEXT NOT NULL,
 "age" INTEGER NOT NULL,
 PRIMARY KEY("id")
);
\`\`\`

And for SQLite:

\`\`\`sql
CREATE TABLE IF NOT EXISTS "Person"(
 "id" INTEGER PRIMARY KEY AUTOINCREMENT,
 "first\_name" TEXT NOT NULL,
 "last\_name" TEXT NOT NULL,
 "age" INTEGER NOT NULL
);
\`\`\`

When you insert an object with an auto-incrementing primary key, you do not need to provide a value for the key field. The database will automatically assign a unique, incrementing value.

\`\`\`cpp
auto homer = Person{.first\_name = "Homer", .last\_name = "Simpson", .age = 45};
// The 'id' field is not set.

// After writing to the database and reading it back, the 'id' will be populated.
auto people = std::vector({homer});
auto result = conn.and\_then(sqlgen::write(std::ref(people)))
 .and\_then(sqlgen::read>())
 .value();

// result\[0\].id will now have a value, for instance 1.
\`\`\`

\### Assignment and Access

Assign values to primary key fields:

\`\`\`cpp
const auto person = People{
 .first\_name = "Homer",
 .last\_name = "Simpson",
 .age = 45
};
\`\`\`

Access the underlying value using any of these methods:

\`\`\`cpp
person.first\_name();
person.first\_name.get();
person.first\_name.value();
\`\`\`

\## Notes

\- The template parameter specifies the type of the primary key field
\- Primary key fields are automatically marked as NOT NULL in the generated SQL
\- Auto-incrementing primary keys must have an integral type.
\- The class supports:
 \- Direct value assignment
 \- Multiple access methods for the underlying value
 \- Reflection for SQL operations
 \- Move and copy semantics
\- Primary keys can be used with any supported SQL data type