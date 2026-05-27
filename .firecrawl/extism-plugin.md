[Skip to main content](https://extism.org/docs/concepts/plug-in/#docusaurus_skipToContent_fallback)

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
- Plug-in

On this page

# What is a Plug-in?

A plug-in is a code module that conforms to the interface of your [Plug-in System](https://extism.org/docs/concepts/plug-in-system). In Extism, a plug-in is ultimately a WebAssembly Module that can be created using one of our [PDKs](https://extism.org/docs/concepts/pdk) in the language of your choice.

WebAssembly modules, `.wasm files`, are a binary representation of your code module. Like all programming languages, WebAssembly modules are composed of functions. Some of these functions are internal, some can be _exported_ to your application so that you can invoke them.

## Exports [​](https://extism.org/docs/concepts/plug-in/\#exports "Direct link to Exports")

You can think of them like a JavaScript module. Functions defined in the file can't be seen or invoked from the outside. It must export a function to be used:

```javascript
// this const and function are private
const VOWELS = "aeiou";
function isVowel(char) {
    return VOWELS.includes(char)
}

// this function is exported and can be invoked from outside
export function countVowels(input) {
  return input
      .split('')
      .reduce((count, char) =>
          count + isVowel(char) ? 1 : 0, 0)
}
```

Of course, what is unique about Extism plug-ins is the language you wrote the plug-in in doesn't matter. We can use any [Host SDK](https://extism.org/docs/concepts/host-sdk) to load these modules and call these exports.

## Imports [​](https://extism.org/docs/concepts/plug-in/\#imports "Direct link to Imports")

Like JavaScript modules, WebAssembly modules also have the ability to _import_ functions. This is useful because WebAssembly code, on its own, is basically a sandboxed calculator. It can only see the world inside its own memory and doesn't have access to the underlying system. For example, it cannot read from the file system or from the processes memory, or anything that would require a syscall.

So any of this functionality must be imported and implemented by the application. Though, you do not need to implement these yourself. Extism provides the module with some of its own imports to facilitate communication. There are also standards like [WASI](https://wasi.dev/) which can give your module some of the POSIX-like capabilities you expect in a normal programming environment (like reading files or doing I/O).

That said, these imports can be very powerful as they allow you to selectively imbue your plug-ins with the capabilities and APIs of your application itself. This is what [Host Functions](https://extism.org/docs/concepts/host-functions) offer. Like the module has the ability to export a function in a language agnostic way, host functions give you a way to import a function into your plug-in in a language agnostic way. If my application is written in python for example, I could pass a python host function to a plug-in that can say read from my database, or call internal APIs. Host Functions truly unlock the power of a plug-in system as they can, selectively, give your plug-in the same capabilities as your internal application code.

[Previous\\
\\
Plug-in System](https://extism.org/docs/concepts/plug-in-system) [Next\\
\\
Host SDKs](https://extism.org/docs/concepts/host-sdk)

- [Exports](https://extism.org/docs/concepts/plug-in/#exports)
- [Imports](https://extism.org/docs/concepts/plug-in/#imports)

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