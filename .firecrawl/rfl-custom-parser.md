[Skip to content](https://rfl.getml.com/custom_parser/#custom-parsers)

[reflect-cpp](https://rfl.getml.com/ "reflect-cpp")

reflect-cpp



Custom parsers for your classes



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




    - [Custom classes](https://rfl.getml.com/concepts/custom_classes/)
    - [ ]


       Custom parsers for your classes



       [Custom parsers for your classes](https://rfl.getml.com/custom_parser/)
       Page contents


      - [rfl::Reflector](https://rfl.getml.com/custom_parser/#rflreflector)
      - [rfl::parsing::CustomParser](https://rfl.getml.com/custom_parser/#rflparsingcustomparser)
      - [Example](https://rfl.getml.com/custom_parser/#example)
      - [Implement the Parser template](https://rfl.getml.com/custom_parser/#implement-the-parser-template)

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


- [rfl::Reflector](https://rfl.getml.com/custom_parser/#rflreflector)
- [rfl::parsing::CustomParser](https://rfl.getml.com/custom_parser/#rflparsingcustomparser)
- [Example](https://rfl.getml.com/custom_parser/#example)
- [Implement the Parser template](https://rfl.getml.com/custom_parser/#implement-the-parser-template)

1. [Welcome](https://rfl.getml.com/)
2. [Documentation](https://rfl.getml.com/docs-readme/)
3. [Custom classes](https://rfl.getml.com/concepts/custom_classes/)

# Custom parsers [¶](https://rfl.getml.com/custom_parser/\#custom-parsers)

## `rfl::Reflector` [¶](https://rfl.getml.com/custom_parser/\#rflreflector)

If you absolutely do not want to (or are unable to) make any changes to your
original classes whatsoever, you can create a Reflector template specialization
for your type:

```
namespace rfl {
template <>
struct Reflector<Person> {
  struct ReflType {
    std::string first_name;
    std::string last_name;
  };

  static Person to(const ReflType& v) noexcept {
    return {v.first_name, v.last_name};
  }

  static ReflType from(const Person& v) {
    return {v.first_name, v.last_name};
  }
};
}
```

One way to help make sure that your `ReflType` is kept up to date with your
original class is to use the `rfl::num_fields<T>` utility to implement a compile-
time assertion to verify that they have the same number of fields. The
`rfl::num_fields<T>` utility can be used even in cases where the original
class is too complex for `reflect-cpp`'s default reflection logic or
`rfl::to_view()` to be able to handle.

```
namespace rfl {
template <>
struct Reflector<Person> {
  struct ReflType {
    std::string first_name;
    std::string last_name;
  };
  static_assert(rfl::num_fields<ReflType> == rfl::num_fields<Person>,
    "ReflType and actual type must have the same number of fields");
  // ...
```

It's also fine to define just the `from` method when the original class is
only written, or `to` when the original class is only read:

```
// This can only be used for writing.
namespace rfl {
template <>
struct Reflector<Person> {
  struct ReflType {
    std::string first_name;
    std::string last_name;
  };

  static ReflType from(const Person& v) {
    return {v.first_name, v.last_name};
  }
};
}
```

Note that the `ReflType` does not have to be a struct. For instance, if you have
a custom type called `MyCustomType` that you want to be serialized as a string,
you can do the following:

```
namespace rfl {
template <>
struct Reflector<MyCustomType> {
  using ReflType = std::string;

  static MyCustomType to(const ReflType& str) noexcept {
    return MyCustomType::from_string(str);
  }

  static ReflType from(const MyCustomType& v) {
    return v.to_string();
  }
};
}
```

## `rfl::parsing::CustomParser` [¶](https://rfl.getml.com/custom_parser/\#rflparsingcustomparser)

Alternatively, you can implement a custom parser using `rfl::parsing::CustomParser`.

In order to do so, you must do the following:

You must create a helper struct that _can_ be parsed. The helper struct must fulfill the following
conditions:

1) It must contain a static method called `from_class` that takes your original class as an input and returns the helper struct. This method must not throw an exception.
2) (Optional) It must contain a method called `to_class` that transforms the helper struct into your original class. This method may throw an exception, if you want to. If you can directly construct your custom class from the field values in the order they were declared in the helper struct, you do not have to write a `to_class` method.

You can then implement a custom parser for your class like this:

```
namespace rfl::parsing {

template <class ReaderType, class WriterType, class ProcessorsType>
struct Parser<ReaderType, WriterType, YourOriginalClass, ProcessorsType>
    : public CustomParser<ReaderType, WriterType, ProcessorsType, YourOriginalClass,
                          YourHelperStruct> {};

}  // namespace rfl::parsing
```

## Example [¶](https://rfl.getml.com/custom_parser/\#example)

Suppose your original class looks like this:

```
struct Person {
    Person(const std::string& _first_name, const std::string& _last_name,
           const int _age)
        : first_name_(_first_name), last_name_(_last_name), age_(_age) {}

    const auto& first_name() const { return first_name_; }

    const auto& last_name() const { return last_name_; }

    auto age() const { return age_; }

   private:
    std::string first_name_;
    std::string last_name_;
    int age_;
};
```

You can then write a helper struct:

```
struct PersonImpl {
    rfl::Rename<"firstName", std::string> first_name;
    rfl::Rename<"lastName", std::string> last_name;
    int age;

    // 1) Static method that takes your original class as an input and
    //    returns the helper struct.
    //    MUST NOT THROW AN EXCEPTION!
    static PersonImpl from_class(const Person& _p) noexcept {
        return PersonImpl{.first_name = _p.first_name(),
                          .last_name = _p.last_name(),
                          .age = _p.age()};
    }

    // 2) Const method called `to_class` that transforms the helper struct
    //    into your original class.
    //    In this case, the `to_class` method is actually optional, because
    //    you can directly create Person from the field values.
    Person to_class() const { return Person(first_name(), last_name(), age); }
};
```

You then implement the custom parser:

```
namespace rfl::parsing {

template <class ReaderType, class WriterType, class ProcessorsType>
struct Parser<ReaderType, WriterType, Person, ProcessorsType>
    : public CustomParser<ReaderType, WriterType, ProcessorsType, Person, PersonImpl> {};

}  // namespace rfl::parsing
```

Now your custom class is fully supported by reflect-cpp. So for instance, you could parse it
inside a vector:

```
const auto people = rfl::json::read<std::vector<Person>>(json_str).value();
```

As we have noted, in this particular example, the `Person` class can be constructed from the field values in
`PersonImpl` in the exact same order they were declared in `PersonImpl`. So we can drop the `.to_class` method:

```
struct PersonImpl {
    rfl::Rename<"firstName", std::string> first_name;
    rfl::Rename<"lastName", std::string> last_name;
    int age;

    static PersonImpl from_class(const Person& _p) noexcept {
        return PersonImpl{.first_name = _p.first_name(),
                          .last_name = _p.last_name(),
                          .age = _p.age()};
    }
};
```

## Implement the `Parser` template [¶](https://rfl.getml.com/custom_parser/\#implement-the-parser-template)

You can also directly implement the Parser template for your type.
This might be beneficial when you have a third-party container type
that behaves like standard containers.

In our example, we are implementing the template for `gtl::flat_hash_map`,
but the approach should also work for similar boost containers.

```
namespace rfl {
namespace parsing {

template <class K, class V, class Hash, class KeyEqual, class Allocator>
class is_map_like<gtl::flat_hash_map<K, V, Hash, KeyEqual, Allocator>> : public std::true_type {};

template <class R, class W, class T, class Hash, class KeyEqual, class ProcessorsType>
  requires AreReaderAndWriter<R, W, gtl::flat_hash_map<std::string, T, Hash>>
struct Parser<R, W, gtl::flat_hash_map<std::string, T, Hash, KeyEqual>, ProcessorsType>
    : public MapParser<R, W, gtl::flat_hash_map<std::string, T, Hash, KeyEqual>, ProcessorsType> {};

template <class R, class W, typename K, typename V, class Hash, class KeyEqual, class Allocator, class ProcessorsType>
  requires AreReaderAndWriter<R, W, gtl::flat_hash_map<K, V, Hash, KeyEqual, Allocator>>
struct Parser<R, W, gtl::flat_hash_map<K, V, Hash, KeyEqual, Allocator>, ProcessorsType>
    : public VectorParser<R, W, gtl::flat_hash_map<K, V, Hash, KeyEqual, Allocator>, ProcessorsType> {};

}
}
```

Back to top



Made with
[Material for MkDocs](https://squidfunk.github.io/mkdocs-material/)

[github.com](https://github.com/getml "github.com")[www.youtube.com](https://www.youtube.com/@code17-gmbh "www.youtube.com")[www.linkedin.com](https://www.linkedin.com/company/code17-gmbh "www.linkedin.com")