[Skip to main content](https://ziggit.dev/t/creating-cross-platform-plugin-system/8099#main-container)

[![Ziggit](https://ziggit.dev/uploads/default/original/2X/a/a65087dbc73e8fc2751c8ff1eebb91d0922b6b27.png)](https://ziggit.dev/)

Sign UpLog In

- ​


- [Topics](https://ziggit.dev/latest "All topics")
- [Docs](https://ziggit.dev/docs "Explore documentation topics")
- More


Categories


- [Brainstorming](https://ziggit.dev/c/brainstorming/9 "A place to discuss ideas and informal proposals about anything Zig-related.")
- [Explain](https://ziggit.dev/c/explain/13 "Looking to understand more about Zig? You’re in the right place! The Explain category is dedicated to free-flowing discussions about the Zig language and its internals, best practices, surrounding ecosystem, and other Zig related topics.")
- [Help](https://ziggit.dev/c/help/6 "Post all your Zig related questions here and see how quickly another community member can help you find the answer.")
- [Showcase](https://ziggit.dev/c/showcase/7 "Post your latest Zig project and let the community know what you’ve been working on.")
- [All categories](https://ziggit.dev/categories)

Tags


- [build-system](https://ziggit.dev/tag/build-system "")
- [language](https://ziggit.dev/tag/language "")
- [memory-management](https://ziggit.dev/tag/memory-management "")
- [standard-library](https://ziggit.dev/tag/standard-library "")
- [All tags](https://ziggit.dev/tags)

​


​


Ziggit is a forum for those interested in, or who are currently programing in the [Zig Programming Language](https://ziglang.org/). We hope you find what you’re looking for, or help others to do just that. Feel free to post any forum-related issues in the _Site Feedback_ category. Once again, welcome to Ziggit and thanks for being part of the Zig community.

# [Creating Cross Platform Plugin System](https://ziggit.dev/t/creating-cross-platform-plugin-system/8099)

[Challenge](https://ziggit.dev/c/challenge/11)

[c](https://ziggit.dev/tag/c), [compiler](https://ziggit.dev/tag/compiler), [cross-compile](https://ziggit.dev/tag/cross-compile)

You have selected **0** posts.

[select all](https://ziggit.dev/t/creating-cross-platform-plugin-system/8099)

[cancel selecting](https://ziggit.dev/t/creating-cross-platform-plugin-system/8099)

518
views
1
link


[![](https://ziggit.dev/user_avatar/ziggit.dev/cypherpunksamurai/48/4191_2.png)2](https://ziggit.dev/u/CypherpunkSamurai "CypherpunkSamurai")

[![](https://ziggit.dev/user_avatar/ziggit.dev/pierrelgol/48/6029_2.png)](https://ziggit.dev/u/pierrelgol "pierrelgol")

[![](https://ziggit.dev/user_avatar/ziggit.dev/pachde/48/2485_2.png)](https://ziggit.dev/u/pachde "pachde")

[![](https://ziggit.dev/user_avatar/ziggit.dev/tobyjaffey/48/3790_2.png)](https://ziggit.dev/u/tobyjaffey "tobyjaffey")

[Jan 2025](https://ziggit.dev/t/creating-cross-platform-plugin-system/8099/1 "Jump to the first post")

2 / 5


Jan 2025


[Jan 2025](https://ziggit.dev/t/creating-cross-platform-plugin-system/8099/5)

## post by CypherpunkSamurai on Jan 23, 2025

[![](https://ziggit.dev/user_avatar/ziggit.dev/cypherpunksamurai/48/4191_2.png)](https://ziggit.dev/u/cypherpunksamurai)

​

0

​


[CypherpunkSamurai](https://ziggit.dev/u/cypherpunksamurai)

[Jan 2025](https://ziggit.dev/t/creating-cross-platform-plugin-system/8099 "Post date")

Hello Zig community,

> Note: Before you read the post I want you to please excuse my knowledge on zig, compilers and shared libraries. I know I’m not great and im happy to learn a lot from you all ![:smiley:](https://ziggit.dev/images/emoji/twitter/smiley.png?v=12)

Ever since I learnt go in 2020 i’ve had the itch to make a cross platform plugin system in golang that compiles to respective platform as shared libraries and allows plug-and-play like capabilities.

Think of a RPC HTTP Server that allows you to add more features by just dragging a .dll file (or a .so for Unix systems) into a “plugins” folder.

I’ve tried to replicate it in golang using shared library compilation. Go also has a plugin build mode, but it’s still not compatible with Windows.

Real Question:

Just my thoughts and 2 cents, would it be possible to build a plugin system that allows us to have portable plug-and-play features in zig? If so how should we/one build it? I would like you guys opinions.


1


​


​


​


I think without a specific idea about what this plugin system should be capable to handle, it is difficult to answer. For a "be able to do anything" system, I would expect it to support communication between plugins, handling load/initialization order, handling version conflicts, handling errors, possibly error reporting, optional dependencies, permissions and capabilities, optional sandboxing via something like wasm (or something similar) for untrusted code.
– [Sze](https://ziggit.dev/u/Sze)Jan 23, 2025

518
views
1
link


[![](https://ziggit.dev/user_avatar/ziggit.dev/cypherpunksamurai/48/4191_2.png)2](https://ziggit.dev/u/CypherpunkSamurai "CypherpunkSamurai")

[![](https://ziggit.dev/user_avatar/ziggit.dev/pierrelgol/48/6029_2.png)](https://ziggit.dev/u/pierrelgol "pierrelgol")

[![](https://ziggit.dev/user_avatar/ziggit.dev/pachde/48/2485_2.png)](https://ziggit.dev/u/pachde "pachde")

[![](https://ziggit.dev/user_avatar/ziggit.dev/tobyjaffey/48/3790_2.png)](https://ziggit.dev/u/tobyjaffey "tobyjaffey")

## post by pierrelgol on Jan 23, 2025

4 Answers

Activity
VotesActivity

[![](https://ziggit.dev/user_avatar/ziggit.dev/pierrelgol/48/6029_2.png)](https://ziggit.dev/u/pierrelgol)

​
1
​


[pierrelgol](https://ziggit.dev/u/pierrelgol)

[Jan 2025](https://ziggit.dev/t/creating-cross-platform-plugin-system/8099/2 "Post date")

Hi welcome to the forum :). I’m no expert, but my guess would be that loading/unloading plugins, and monitoring a folder or events for loading/unloading plugins should be quite straightforward to achieve. As for the “interface” for the plugins that’s another story, If you need something quite static, defining your own VTable (for example you can look at something like the Allocator VTable) of optional function pointer should do the trick, for something more dynamic tho I don’t have any idea how you could do it, It’s probably doable but It would require more knowledge than I have. Have you considered embedding a dynamic language, like Lua ? It could potentially solve your problem if performance is not absolutely critical.


1


​


​


## post by pachde on Jan 23, 2025

[![](https://ziggit.dev/user_avatar/ziggit.dev/pachde/48/2485_2.png)](https://ziggit.dev/u/pachde)

​

0

​


[pachde](https://ziggit.dev/u/pachde)

[Jan 2025](https://ziggit.dev/t/creating-cross-platform-plugin-system/8099/3 "Post date")

Native code plugins (versus interpreters/bytecode vms) by their nature are separate compilation units, and Zig is a single-compilation-unit language, and all communication between host and plugin must be via a C interface.

The other half is a how you discover and load the plugins which as previously noted, is easily achievable.

In some cases you’d have a strategy where your plugin convention is to use an array of function pointers communicated by a single external symbol or function. Otherwise, the caller must handle the binding by lookups specific to the OS binary format in question (ELF, COFF…).

​


​


​


There's no technical reason for Zig not to adapt an FFI that supports Zig-specific constructs and I'd love to see it! But there's a lot of work I imagine and perhaps not too important atm.
– [ajoino](https://ziggit.dev/u/ajoino)Jan 23, 2025

## post by tobyjaffey on Jan 24, 2025

[![](https://ziggit.dev/user_avatar/ziggit.dev/tobyjaffey/48/3790_2.png)](https://ziggit.dev/u/tobyjaffey)

​

0

​


[tobyjaffey](https://ziggit.dev/u/tobyjaffey)

[Jan 2025](https://ziggit.dev/t/creating-cross-platform-plugin-system/8099/4 "Post date")

Wasm could be a good fit here. Many modern languages, including zig, support compiling to wasm and there are a range of VMs you could embed in your app to handle running plugin code.


2


​


​


## post by CypherpunkSamurai on Jan 24, 2025

[![](https://ziggit.dev/user_avatar/ziggit.dev/cypherpunksamurai/48/4191_2.png)](https://ziggit.dev/u/cypherpunksamurai)

​

0

​


[CypherpunkSamurai](https://ziggit.dev/u/cypherpunksamurai)

[Jan 2025](https://ziggit.dev/t/creating-cross-platform-plugin-system/8099/5 "Post date")

[@pierrelgol](https://ziggit.dev/u/pierrelgol) you’re right, when it comes to performance having a VTable / Exported Function Table should be enough.

[@ajoino](https://ziggit.dev/u/ajoino) I’m assuming this to be a lib, like a library that allows us to have modular codebase and split the functionalities to different pluggable module each. Something that comes to my mind is yt-dlp (youtube-dl) rewrite in zig. It would support youtube and other urls, but you can support more websites by dragging and dropping a .DLL or a .so file

Here’s something similar in cpp:

[github.com](https://github.com/caiorss/sample-cpp-plugin)

![](https://ziggit.dev/uploads/default/optimized/2X/2/2e8d6cadf93c352892963f0c0e002e3ca058bc85_2_690x344.png)

### [GitHub - caiorss/sample-cpp-plugin: C++ cross-platform plugin architechture...](https://github.com/caiorss/sample-cpp-plugin)

C++ cross-platform plugin architechture demonstration

​


​


Reply

[Powered by Discourse](https://discourse.org/powered-by)

Invalid date

Invalid date