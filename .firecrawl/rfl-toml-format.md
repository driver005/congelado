[Skip to content](https://rfl.getml.com/supported_formats/toml/#toml)

[reflect-cpp](https://rfl.getml.com/ "reflect-cpp")

reflect-cpp



TOML



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
- [x]


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
  - [ ]


     TOML



     [TOML](https://rfl.getml.com/supported_formats/toml/)
     Page contents


    - [Reading and writing](https://rfl.getml.com/supported_formats/toml/#reading-and-writing)
    - [Loading and saving](https://rfl.getml.com/supported_formats/toml/#loading-and-saving)
    - [Reading from and writing into streams](https://rfl.getml.com/supported_formats/toml/#reading-from-and-writing-into-streams)
    - [Custom constructors](https://rfl.getml.com/supported_formats/toml/#custom-constructors)

  - [UBJSON](https://rfl.getml.com/supported_formats/ubjson/)
  - [XML](https://rfl.getml.com/supported_formats/xml/)
  - [YAML](https://rfl.getml.com/supported_formats/yaml/)
  - [yas](https://rfl.getml.com/supported_formats/yas/)
  - [Custom Format](https://rfl.getml.com/supported_formats/supporting_your_own_format/)

- [Installation](https://rfl.getml.com/install/)
- [ ]


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

  - [ ]


     Custom classes






     Custom classes




    - [Custom classes](https://rfl.getml.com/concepts/custom_classes/)
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


- [Reading and writing](https://rfl.getml.com/supported_formats/toml/#reading-and-writing)
- [Loading and saving](https://rfl.getml.com/supported_formats/toml/#loading-and-saving)
- [Reading from and writing into streams](https://rfl.getml.com/supported_formats/toml/#reading-from-and-writing-into-streams)
- [Custom constructors](https://rfl.getml.com/supported_formats/toml/#custom-constructors)

1. [Welcome](https://rfl.getml.com/)
2. [Supported Formats](https://rfl.getml.com/supported_formats/avro/)

# TOML [¶](https://rfl.getml.com/supported_formats/toml/\#toml)

For TOML support, you must also include the header `<rfl/toml.hpp>` and include the [toml++](https://github.com/marzer/tomlplusplus) library.
Furthermore, when compiling reflect-cpp, you need to pass `-DREFLECTCPP_TOML=ON` to cmake. If you are using vcpkg or Conan, there
should be an appropriate feature (vcpkg) or option (Conan) that will abstract this away for you.

## Reading and writing [¶](https://rfl.getml.com/supported_formats/toml/\#reading-and-writing)

Suppose you have a struct like this:

```
struct Person {
    rfl::Rename<"firstName", std::string> first_name;
    rfl::Rename<"lastName", std::string> last_name;
    rfl::Timestamp<"%Y-%m-%d"> birthday;
    std::vector<Person> children;
};
```

You can parse TOML strings like this:

```
const rfl::Result<Person> result = rfl::toml::read<Person>(toml_string);
```

A `person` can be serialized like this:

```
const auto person = Person{...};
const std::string toml_string = rfl::toml::write(person);
```

## Loading and saving [¶](https://rfl.getml.com/supported_formats/toml/\#loading-and-saving)

You can also load and save to disc using a very similar syntax:

```
const rfl::Result<Person> result = rfl::toml::load<Person>("/path/to/file.toml");

const auto person = Person{...};
rfl::toml::save("/path/to/file.toml", person);
```

## Reading from and writing into streams [¶](https://rfl.getml.com/supported_formats/toml/\#reading-from-and-writing-into-streams)

You can also read from and write into any `std::istream` and `std::ostream` respectively.

```
const rfl::Result<Person> result = rfl::toml::read<Person>(my_istream);

const auto person = Person{...};
rfl::toml::write(person, my_ostream);
```

Note that `std::cout` is also an ostream, so this works as well:

```
rfl::toml::write(person, std::cout) << std::endl;
```

## Custom constructors [¶](https://rfl.getml.com/supported_formats/toml/\#custom-constructors)

One of the great things about C++ is that it gives you control over
when and how you code is compiled.

For large and complex systems of structs, it is often a good idea to split up
your code into smaller compilation units. You can do so using custom constructors.

For the TOML format, these must be a static function on your struct or class called
`from_toml_obj` that take a `rfl::toml::Reader::InputVarType` as input and return
the class or the class wrapped in `rfl::Result`.

In your header file you can write something like this:

```
struct Person {
    rfl::Rename<"firstName", std::string> first_name;
    rfl::Rename<"lastName", std::string> last_name;
    rfl::Timestamp<"%Y-%m-%d"> birthday;

    using TOMLVar = typename rfl::toml::Reader::InputVarType;
    static rfl::Result<Person> from_toml_obj(const TOMLVar& _obj);
};
```

And in your source file, you implement `from_toml_obj` as follows:

```
rfl::Result<Person> Person::from_toml_obj(const TOMLVar& _obj) {
    const auto from_nt = [](auto&& _nt) {
        return rfl::from_named_tuple<Person>(std::move(_nt));
    };
    return rfl::toml::read<rfl::named_tuple_t<Person>>(_obj)
        .transform(from_nt);
}
```

This will force the compiler to only compile the TOML parsing when the
source file is compiled.

Back to top



Made with
[Material for MkDocs](https://squidfunk.github.io/mkdocs-material/)

[github.com](https://github.com/getml "github.com")[www.youtube.com](https://www.youtube.com/@code17-gmbh "www.youtube.com")[www.linkedin.com](https://www.linkedin.com/company/code17-gmbh "www.linkedin.com")