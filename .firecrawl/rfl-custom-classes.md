[Skip to content](https://rfl.getml.com/concepts/custom_classes/#custom-classes)

[reflect-cpp](https://rfl.getml.com/ "reflect-cpp")

reflect-cpp



Custom classes



Type to start searching

[getml/reflect-cpp\\
\\
\\
- v0.25.0\\
- 1.9k\\
- 181](https://github.com/getml/reflect-cpp "Go to repository")

- [Welcome](https://rfl.getml.com/)
- [Supported Formats](https://rfl.getml.com/supported_formats/avro/)
- [Installation](https://rfl.getml.com/install/)
- [Documentation](https://rfl.getml.com/docs-readme/)
- [Contributing](https://rfl.getml.com/contributing/)

[reflect-cpp](https://rfl.getml.com/ "reflect-cpp")
reflect-cpp


[getml/reflect-cpp\\
\\
\\
- v0.25.0\\
- 1.9k\\
- 181](https://github.com/getml/reflect-cpp "Go to repository")

- [Welcome](https://rfl.getml.com/)
- [ ]


Supported Formats






Supported Formats




  - [Avro](https://rfl.getml.com/supported_formats/avro/)
  - [Boost Serialization](https://rfl.getml.com/supported_formats/boost_serialization/)
  - [BSON](https://rfl.getml.com/supported_formats/bson/)
  - [Cap'n Proto](https://rfl.getml.com/supported_formats/capnproto/)
  - [CBOR](https://rfl.getml.com/supported_formats/cbor/)
  - [Cereal](https://rfl.getml.com/supported_formats/cereal/)
  - [CSV](https://rfl.getml.com/supported_formats/csv/)
  - [FlexBuffers](https://rfl.getml.com/supported_formats/flexbuffers/)
  - [JSON](https://rfl.getml.com/supported_formats/json/)
  - [MessagePack](https://rfl.getml.com/supported_formats/msgpack/)
  - [Parquet](https://rfl.getml.com/supported_formats/parquet/)
  - [TOML](https://rfl.getml.com/supported_formats/toml/)
  - [UBJSON](https://rfl.getml.com/supported_formats/ubjson/)
  - [XML](https://rfl.getml.com/supported_formats/xml/)
  - [YAML](https://rfl.getml.com/supported_formats/yaml/)
  - [yas](https://rfl.getml.com/supported_formats/yas/)
  - [Custom Format](https://rfl.getml.com/supported_formats/supporting_your_own_format/)

- [Installation](https://rfl.getml.com/install/)
- [x]


Documentation






Documentation




  - [Documentation](https://rfl.getml.com/docs-readme/)
  - [ ]


     The basics






     The basics




    - [Structs](https://rfl.getml.com/concepts/structs/)
    - [Optional fields](https://rfl.getml.com/optional_fields/)
    - [Default values](https://rfl.getml.com/default_val/)
    - [Struct flattening](https://rfl.getml.com/flatten_structs/)
    - [Processors](https://rfl.getml.com/concepts/processors/)
    - [The rfl::Field-syntax](https://rfl.getml.com/concepts/field_syntax/)
    - [String literals](https://rfl.getml.com/literals/)
    - [Enums](https://rfl.getml.com/enums/)
    - [std::variant and rfl::TaggedUnion](https://rfl.getml.com/variants_and_tagged_unions/)
    - [rfl::Box and rfl::Ref](https://rfl.getml.com/rfl_ref/)
    - [rfl::Timestamp](https://rfl.getml.com/timestamps/)
    - [rfl::Skip](https://rfl.getml.com/rfl_skip/)
    - [rfl::Commented](https://rfl.getml.com/commented/)
    - [rfl::Result](https://rfl.getml.com/result/)
    - [Standard containers](https://rfl.getml.com/standard_containers/)
    - [C arrays and inheritance](https://rfl.getml.com/c_arrays_and_inheritance/)
    - [rfl::Bytestring](https://rfl.getml.com/bytestring/)
    - [rfl::Binary, rfl::Hex and rfl::Oct](https://rfl.getml.com/number_systems/)
    - [std::atomic, std::atomic\_flag](https://rfl.getml.com/atomic/)

  - [ ]


     Validation






     Validation




    - [Regex patterns](https://rfl.getml.com/patterns/)
    - [Validating numbers](https://rfl.getml.com/validating_numbers/)
    - [Composing validators](https://rfl.getml.com/composing_validators/)
    - [Size validation](https://rfl.getml.com/size_validation/)
    - [JSON schema](https://rfl.getml.com/json_schema/)
    - [Enum descriptions](https://rfl.getml.com/enum_descriptions/)

  - [ ]


     Generic elements






     Generic elements




    - [rfl::Object](https://rfl.getml.com/object/)
    - [rfl::Generic](https://rfl.getml.com/generic/)
    - [rfl::ExtraFields](https://rfl.getml.com/extra_fields/)

  - [x]


     Custom classes






     Custom classes




    - [ ]


       Custom classes



       [Custom classes](https://rfl.getml.com/concepts/custom_classes/)
       Page contents


      - [Example 1: Using an Impl struct](https://rfl.getml.com/concepts/custom_classes/#example-1-using-an-impl-struct)
      - [Example 2: Matching variables, the safe way](https://rfl.getml.com/concepts/custom_classes/#example-2-matching-variables-the-safe-way)
      - [Example 3: Matching variables, the unsafe way](https://rfl.getml.com/concepts/custom_classes/#example-3-matching-variables-the-unsafe-way)

    - [Custom parsers for your classes](https://rfl.getml.com/custom_parser/)

  - [ ]


     Useful helper functions and classes






     Useful helper functions and classes




    - [rfl::replace](https://rfl.getml.com/replace/)
    - [rfl::as](https://rfl.getml.com/as/)
    - [rfl::NamedTuple](https://rfl.getml.com/named_tuple/)
    - [rfl::Tuple](https://rfl.getml.com/rfl_tuple/)
    - [rfl::to\_view](https://rfl.getml.com/to_view/)

  - [ ]


     Advanced topics






     Advanced topics




    - [Custom Format](https://rfl.getml.com/supported_formats/supporting_your_own_format/)
    - [Maintaining backwards compatability](https://rfl.getml.com/backwards_compatability/)
    - [Benchmarks](https://rfl.getml.com/benchmarks/)

- [Contributing](https://rfl.getml.com/contributing/)

Page contents


- [Example 1: Using an Impl struct](https://rfl.getml.com/concepts/custom_classes/#example-1-using-an-impl-struct)
- [Example 2: Matching variables, the safe way](https://rfl.getml.com/concepts/custom_classes/#example-2-matching-variables-the-safe-way)
- [Example 3: Matching variables, the unsafe way](https://rfl.getml.com/concepts/custom_classes/#example-3-matching-variables-the-unsafe-way)

1. [Welcome](https://rfl.getml.com/)
2. [Documentation](https://rfl.getml.com/docs-readme/)
3. [Custom classes](https://rfl.getml.com/concepts/custom_classes/)

# Custom classes [¶](https://rfl.getml.com/concepts/custom_classes/\#custom-classes)

Reflection implies that all your fields are public. But in object-oriented programming, you often don't want
that. If your class is more than a trivial, behaviorless struct, you often want to make your fields private.

If you want your class to be supported by reflect-cpp, it needs to have the following:

1) It needs to publicly define a type called `ReflectionType` using `using` or `typedef`.
2) It needs to have a constructor that accepts your `ReflectionType` as an argument.
3) It needs to contain a method called `reflection` that returns said `ReflectionType` (or a reference thereto).

If you class fulfills these three conditions, then it is fully supported by all serialization and deserialization
routines in reflect-cpp.

Please be aware that due to limitations of the Avro format, it is a good idea to always have a struct as your
`ReflectionType` when using Avro to avoid infinite recursions.

If you absolutely do not want to make any changes to your original class, you can implement a [custom parser](https://github.com/getml/reflect-cpp/blob/main/docs/custom_parser.md).

## Example 1: Using an Impl struct [¶](https://rfl.getml.com/concepts/custom_classes/\#example-1-using-an-impl-struct)

```
struct PersonImpl {
    rfl::Rename<"firstName", std::string> first_name;
    rfl::Rename<"lastName", std::string> last_name;
    int age;
};

class Person {
    public:
      // 1) Publicly define `ReflectionType`
      using ReflectionType = PersonImpl;

      // 2) Constructor that accepts your `ReflectionType`
      Person(const PersonImpl& _impl): impl(_impl) {}

      ~Person() = default;

      // 3) Method called `reflection` that returns `ReflectionType`
      const ReflectionType& reflection() const { return impl; }

      // ...add some more methods here...

    private:
        PersonImpl impl;
};
```

## Example 2: Matching variables, the safe way [¶](https://rfl.getml.com/concepts/custom_classes/\#example-2-matching-variables-the-safe-way)

`rfl::Field` is designed in a way that you have to explicitly initialize
every the field (using `rfl::default_value`, if necessary), otherwise
you will get a compile-time error.

A frequent error that happens during serialization/deserialization is that programmers
add a field to their class (`Person` in this example), but forget to update
their serialization routine.

The example as shown below will protect you from any such errors, as all
fields will have to be explicitly initialized, otherwise you will get a
compile-time error. If you add a new field to `Person` you will have to
add it to `PersonImpl` as well and then explicitly initialize it in the
constructor.

Don't worry `operator()` in `rfl::Field` is inlined. There won't be any
runtime overhead.

```
struct PersonImpl {
    rfl::Field<"firstName", std::string> first_name;
    rfl::Field<"lastName", std::string> last_name;
    rfl::Field<"age", int> age;
};

class Person {
    public:
      // 1) Publicly define `ReflectionType`
      using ReflectionType = PersonImpl;

      // 2) Constructor that accepts your `ReflectionType`
      // This as the additional benefit that not only the types,
      // but also the names of the fields will be checked at compile time.
      Person(const PersonImpl& _impl): first_name(_impl.first_name),
          last_name(_impl.last_name), age(_impl.age) {}

      ~Person() = default;

      // 3) Method called `reflection` that returns `ReflectionType`
      ReflectionType reflection() const {
          return PersonImpl{
            .first_name = first_name,
            .last_name = last_name,
            .age = age};
      }

      // ...add some more methods here...

    private:
      rfl::Field<"firstName", std::string> first_name;
      rfl::Field<"lastName", std::string> last_name;
      rfl::Field<"age", int> age;
};
```

## Example 3: Matching variables, the unsafe way [¶](https://rfl.getml.com/concepts/custom_classes/\#example-3-matching-variables-the-unsafe-way)

If, for any reason, you absolutely cannot change the fields
of your class, you have to make sure that all classes are properly
initialized or face runtime errors.

```
struct PersonImpl {
    // ... same as in Example 1 or 2
};

class Person {
    // 1) Publicly define `ReflectionType`
    using ReflectionType = PersonImpl;

    // 2) Constructor that accepts your `ReflectionType`
    Person(const PersonImpl& _impl): first_name(_impl.first_name()),
        last_name(_impl.last_name()), age(_impl.age()) {}

    // ... same as in Example 2

    private:
      std::string first_name;
      std::string last_name;
      int age;
};
```

Back to top



Made with
[Material for MkDocs](https://squidfunk.github.io/mkdocs-material/)

[github.com](https://github.com/getml "github.com")[www.youtube.com](https://www.youtube.com/@code17-gmbh "www.youtube.com")[www.linkedin.com](https://www.linkedin.com/company/code17-gmbh "www.linkedin.com")