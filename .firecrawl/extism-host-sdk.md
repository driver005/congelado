[Skip to main content](https://extism.org/docs/concepts/host-sdk/#docusaurus_skipToContent_fallback)

[🆕 👉 Take Extism to production with ![XTP](https://cdn.prod.website-files.com/65ea17ae827e00e357404e96/65ea18d38b40de5c0647d5b0_xtp-logo.png) 👈 🆕](https://getxtp.com/)

[![Extism](https://extism.org/img/logo-horizontal.png)](https://extism.org/)[Overview](https://extism.org/docs/overview) [Quickstart](https://extism.org/docs/category/quickstart) [Concepts](https://extism.org/docs/category/concepts) [Blog](https://extism.org/blog)

[Extism Playground](https://playground.extism.org/) [GitHub](https://github.com/extism/extism) [Discord](https://discord.gg/cx3usBCWnc)

Search`` `K`

- [Overview](https://extism.org/docs/overview)
- [Quickstart](https://extism.org/docs/category/quickstart)

- [Concepts](https://extism.org/docs/category/concepts)

  - [Plug-in System](https://extism.org/docs/concepts/plug-in-system)
  - [Plug-in](https://extism.org/docs/concepts/plug-in)
  - [Host SDKs](https://extism.org/docs/concepts/host-sdk)
  - [Host Functions](https://extism.org/docs/concepts/host-functions)
  - [Plug-in Development Kits (PDKs)](https://extism.org/docs/concepts/pdk)
  - [Memory](https://extism.org/docs/concepts/memory)
  - [The Manifest](https://extism.org/docs/concepts/manifest)
  - [Configuration](https://extism.org/docs/concepts/configuration)
  - [Runtime APIs](https://extism.org/docs/concepts/runtime-apis)
  - [Testing Plugins](https://extism.org/docs/concepts/testing)
  - [Contributing](https://extism.org/docs/concepts/contributing)
- [Extism CLI](https://extism.org/docs/install)
- [FAQs](https://extism.org/docs/questions)

- [Home page](https://extism.org/)
- [Concepts](https://extism.org/docs/category/concepts)
- Host SDKs

On this page

# Host SDKs

In Extism parlance, we call the application that your plug-ins extend the _host_. e.g: in VS Code, if the extensions are the _plug-ins_ then the editor itself is the _host_.

The library you use to manage and run plug-ins in the host is called a _Host SDK_.

### Usage [​](https://extism.org/docs/concepts/host-sdk/\#usage "Direct link to Usage")

Within your program, you must add a library dependency for a Host SDK in order to execute plug-ins. Extism's officially supported SDKs are published to each language's respective primary package manager, e.g. [Crates.io](https://crates.io/users/extism-bot) for Rust, or [npm](https://www.npmjs.com/org/extism) for Node.js.

- [![Browser](https://extism.org/img/sdk-languages/browser.svg)](https://github.com/extism/js-sdk)
- [![C](https://extism.org/img/sdk-languages/c.svg)](https://github.com/extism/extism/tree/main/libextism)
- [![C++](https://extism.org/img/sdk-languages/cpp.svg)](https://github.com/extism/cpp-sdk)
- [![Elixir](https://extism.org/img/sdk-languages/elixir.svg)](https://github.com/extism/elixir-sdk)
- [![Go](https://extism.org/img/sdk-languages/go.svg)](https://github.com/extism/go-sdk)
- [![Haskell](https://extism.org/img/sdk-languages/haskell.svg)](https://github.com/extism/haskell-sdk)
- [![Java](https://extism.org/img/sdk-languages/java-android.svg)](https://github.com/extism/java-sdk)
- [![.NET](https://extism.org/img/sdk-languages/dotnet.svg)](https://github.com/extism/dotnet-sdk)
- [![Node](https://extism.org/img/sdk-languages/node.svg)](https://github.com/extism/js-sdk)
- [![OCaml](https://extism.org/img/sdk-languages/ocaml.svg)](https://github.com/extism/ocaml-sdk)
- [![PHP](https://extism.org/img/sdk-languages/perl.svg)](https://github.com/extism/perl-sdk)
- [![PHP](https://extism.org/img/sdk-languages/php.svg)](https://github.com/extism/php-sdk)
- [![Python](https://extism.org/img/sdk-languages/python.svg)](https://github.com/extism/python-sdk)
- [![Ruby](https://extism.org/img/sdk-languages/ruby.svg)](https://github.com/extism/ruby-sdk)
- [![Rust](https://extism.org/img/sdk-languages/rust.svg)](https://github.com/extism/extism/tree/main/runtime)
- [![Zig](https://extism.org/img/sdk-languages/zig.svg)](https://github.com/extism/zig-sdk)

If you would like to implement an SDK in another language, please refer to the [Runtime APIs](https://extism.org/docs/concepts/runtime-apis) section to see the functions you will need to write bindings to.

See the [Integrate into your codebase](https://extism.org/docs/quickstart/host-quickstart) section to find a Host SDK in your language, as well as installation instructions to get started.

If you are looking to build a _plug-in_ for a Host program or application, please see the [Write a Plug-in](https://extism.org/docs/concepts/pdk) section for more information about PDKs.

### Need help? [​](https://extism.org/docs/concepts/host-sdk/\#need-help "Direct link to Need help?")

If you've encountered a bug or think something is missing, please open an issue on the [Extism GitHub](https://github.com/extism/extism) repository.

There is an active community on [Discord](https://discord.gg/cx3usBCWnc) where the project maintainers and users can help you. Come hang out!

[Previous\\
\\
Plug-in](https://extism.org/docs/concepts/plug-in) [Next\\
\\
Host Functions](https://extism.org/docs/concepts/host-functions)

- [Usage](https://extism.org/docs/concepts/host-sdk/#usage)
- [Need help?](https://extism.org/docs/concepts/host-sdk/#need-help)

Docs

- [Overview](https://extism.org/docs/overview)
- [Installation](https://extism.org/docs/install)
- [Quickstart](https://extism.org/docs/quickstart/host-quickstart)
- [Write a Plug-in](https://extism.org/docs/quickstart/plugin-quickstart)

Community

- [Extism Improvement Proposals (EIP)](https://github.com/extism/proposals)
- [GitHub Discussions](https://github.com/extism/extism/discussions)
- [Discord](https://discord.gg/cx3usBCWnc)
- [Twitter](https://twitter.com/extism)
- [Stack Overflow](https://stackoverflow.com/questions/tagged/extism)

Commercial Support

- [Dylibso](https://dylib.so/)

© 2026 Dylibso, Inc.