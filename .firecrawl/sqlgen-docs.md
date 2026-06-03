\# sqlgen Documentation

Welcome to the sqlgen documentation. This guide provides detailed information about sqlgen's features and APIs.

\## Core Concepts

\- \[Defining Tables\](defining\_tables.md) - How to define tables using C++ structs
\- \[sqlgen::col\](col.md) - How to represent columns in queries
\- \[sqlgen::literals\](literals.md) - How to use column and table alias literals in queries
\- \[sqlgen::Flatten\](flatten.md) - How to "inherit" fields from other structs
\- \[sqlgen::PrimaryKey\](primary\_key.md) - How to define primary keys in sqlgen
\- \[sqlgen::Result\](result.md) - How sqlgen handles errors and results
\- \[sqlgen::to\_sql\](to\_sql.md) - How to transpile C++ operations to dialect-specific SQL

\## Database I/O

\- \[sqlgen::read\](reading.md) - How to read data from a database
\- \[sqlgen::write\](writing.md) - How to write data to a database

\## Database Operations

\- \[sqlgen::create\_as\](create\_as.md) - How to create tables and views from SELECT queries
\- \[sqlgen::create\_index\](create\_index.md) - How to create an index on a table
\- \[sqlgen::create\_table\](create\_table.md) - How to create a new table
\- \[sqlgen::delete\_from\](delete\_from.md) - How to delete data from a table
\- \[sqlgen::drop\](drop.md) - How to drop a table
\- \[sqlgen::exec\](exec.md) - How to execute raw SQL statements
\- \[sqlgen::group\_by and Aggregations\](group\_by\_and\_aggregations.md) - How generate GROUP BY queries and aggregate data
\- \[sqlgen::inner\_join, sqlgen::left\_join, sqlgen::right\_join, sqlgen::full\_join\](joins.md) - How to join different tables
\- \[sqlgen::insert, sqlgen::insert\_or\_replace\](insert.md) - How to insert data within transactions
\- \[sqlgen::select\_from\](select\_from.md) - How to read data from a database using more complex queries
\- \[sqlgen::unite and sqlgen::unite\_all\](unite.md) - How to combine results from multiple SELECT statements
\- \[sqlgen::update\](update.md) - How to update data in a table

\## Other Operations

\- \[Cache\](cache.md) - How to improve performance with caching.
\- \[Mathematical Operations\](mathematical\_operations.md) - How to use mathematical functions in queries (e.g., abs, ceil, floor, exp, trigonometric functions, round).
\- \[String Operations\](string\_operations.md) - How to manipulate and transform strings in queries (e.g., length, lower, upper, trim, replace, concat).
\- \[Type Conversion Operations\](type\_conversion\_operations.md) - How to convert between types safely in queries (e.g., cast int to double).
\- \[Null Handling Operations\](null\_handling\_operations.md) - How to handle nullable values and propagate nullability correctly (e.g., with coalesce and nullability rules).
\- \[Timestamp and Date/Time Functions\](timestamp\_operations.md) - How to work with timestamps, dates, and times (e.g., extract parts, perform arithmetic, convert formats).
\- \[Enums\](enum.md) - How to work with enums sqlgen

\## Data Types and Validation

\- \[sqlgen::Dynamic\](dynamic.md) - How to define custom SQL types not natively supported by sqlgen
\- \[sqlgen::ForeignKey\](foreign\_key.md) - How to establish referential integrity between tables
\- \[sqlgen::JSON\](json.md) - How to store and work with JSON fields
\- \[sqlgen::Pattern\](pattern.md) - How to add regex pattern validation to avoid SQL injection
\- \[sqlgen::Timestamp\](timestamp.md) - How timestamps work in sqlgen
\- \[sqlgen::Unique\](unique.md) - How to enforce uniqueness constraints on table columns
\- \[sqlgen::Varchar\](varchar.md) - How varchars work in sqlgen

\## Other concepts

\- \[Connection Pool\](connection\_pool.md) - How to manage database connections efficiently
\- \[Transactions\](transactions.md) - How to use transactions for atomic operations
\- \[Views\](views.md) - How to create and manage database views

\## Supported Databases

\- \[DuckDB\](duckdb.md) - How to interact with DuckDB
\- \[MySQL\](mysql.md) - How to interact with MariaDB and MySQL
\- \[PostgreSQL\](postgres.md) - How to interact with PostgreSQL and compatible databases (Redshift, Aurora, Greenplum, CockroachDB, ...)
\- \[SQLite\](sqlite.md) - How to interact with SQLite3

For installation instructions, quick start guide, and usage examples, please refer to the \[main README\](../README.md).