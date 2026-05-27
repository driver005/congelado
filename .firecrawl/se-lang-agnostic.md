# ![site logo](https://softwareengineering.stackexchange.com/Content/Sites/softwareengineering/Img/icon-48.png?v=212c14faefc6)

By clicking “Sign up”, you agree to our [terms of service](https://softwareengineering.stackexchange.com/legal/terms-of-service/public) and acknowledge you have read our [privacy policy](https://softwareengineering.stackexchange.com/legal/privacy-policy).

Sign up with Google

# OR

Email

Password

Sign up

Already have an account? [Log in](https://softwareengineering.stackexchange.com/users/login)

[Skip to main content](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system#content)

#### Stack Exchange Network

Stack Exchange network consists of 183 Q&A communities including [Stack Overflow](https://stackoverflow.com/), the largest, most trusted online community for developers to learn, share their knowledge, and build their careers.


[Visit Stack Exchange](https://stackexchange.com/)

Loading…

1. [Help Center and other resources](https://softwareengineering.stackexchange.com/help "Help Center and other resources")




   - [Tour\\
      \\
      Start here for a quick overview of the site](https://softwareengineering.stackexchange.com/tour)
   - [Help Center\\
      \\
      Detailed answers to any questions you might have](https://softwareengineering.stackexchange.com/help)
   - [Meta\\
      \\
      Discuss the workings and policies of this site](https://softwareengineering.meta.stackexchange.com/)
   - [About Us\\
      \\
      Learn more about Stack Overflow the company, and our products](https://stackoverflow.co/)

2. [A list of all 183 Stack Exchange sites](https://stackexchange.com/ "A list of all 183 Stack Exchange sites")
3. ### [current community](https://softwareengineering.stackexchange.com/)















   - [Software Engineering](https://softwareengineering.stackexchange.com/)

     [help](https://softwareengineering.stackexchange.com/help) [chat](https://chat.stackexchange.com/?tab=site&host=softwareengineering.stackexchange.com)

   - [Software Engineering Meta](https://softwareengineering.meta.stackexchange.com/)

### your communities

[Sign up](https://softwareengineering.stackexchange.com/users/signup?ssrc=site_switcher&returnurl=https%3a%2f%2fsoftwareengineering.stackexchange.com%2fquestions%2f358750%2fmaking-a-language-agnostic-plugin-system) or [log in](https://softwareengineering.stackexchange.com/users/login?ssrc=site_switcher&returnurl=https%3a%2f%2fsoftwareengineering.stackexchange.com%2fquestions%2f358750%2fmaking-a-language-agnostic-plugin-system) to customize your list.

### [more stack exchange communities](https://stackexchange.com/sites)

[company blog](https://stackoverflow.blog/)

5. [Log in](https://softwareengineering.stackexchange.com/users/login?ssrc=head&returnurl=https%3a%2f%2fsoftwareengineering.stackexchange.com%2fquestions%2f358750%2fmaking-a-language-agnostic-plugin-system)
6. [Sign up](https://softwareengineering.stackexchange.com/users/signup?ssrc=head&returnurl=https%3a%2f%2fsoftwareengineering.stackexchange.com%2fquestions%2f358750%2fmaking-a-language-agnostic-plugin-system)

[![Software Engineering](https://softwareengineering.stackexchange.com/Content/Sites/softwareengineering/Img/logo.svg?v=e86f7d5306ae)](https://softwareengineering.stackexchange.com/)

01. [Home](https://softwareengineering.stackexchange.com/)
02. [Questions](https://softwareengineering.stackexchange.com/questions)
03. [Unanswered](https://softwareengineering.stackexchange.com/unanswered)
04. [AI Assist](https://stackoverflow.com/ai-assist)
05. [Tags](https://softwareengineering.stackexchange.com/tags)
07. [Chat](https://chat.stackexchange.com/)
08. [Users](https://softwareengineering.stackexchange.com/users)
10. [Companies](https://stackoverflow.com/jobs/companies?so_medium=softwareengineering&so_source=SiteNav)
2. Stack Internal

Stack Overflow for Teams is now called **Stack Internal**. Bring the best of human thought and AI automation together at your work.


[Try for free](https://stackoverflowteams.com/teams/create/free/?utm_medium=referral&utm_source=softwareengineering-community&utm_campaign=side-bar&utm_content=explore-teams) [Learn more](https://stackoverflow.co/internal/?utm_medium=referral&utm_source=softwareengineering-community&utm_campaign=side-bar&utm_content=explore-teams)

3. Stack Internal

4. Bring the best of human thought and AI automation together at your work.
[Learn more](https://stackoverflow.co/internal/?utm_medium=referral&utm_source=softwareengineering-community&utm_campaign=side-bar&utm_content=explore-teams-compact)

**Stack Internal**

Knowledge at work

Bring the best of human thought and AI automation together at your work.

[Explore Stack Internal](https://stackoverflow.co/internal/?utm_medium=referral&utm_source=softwareengineering-community&utm_campaign=side-bar&utm_content=explore-teams-compact-popover)

# [Making a language agnostic Plugin system](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system)

[Ask Question](https://softwareengineering.stackexchange.com/questions/ask)

Asked8 years, 7 months ago

Modified [6 years ago](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system?lastactivity "2020-05-06 15:20:10Z")

Viewed
3k times


This question shows research effort; it is useful and clear

6

This question does not show any research effort; it is unclear or not useful

Save this question.

[Timeline](https://softwareengineering.stackexchange.com/posts/358750/timeline)

Show activity on this post.

I want to make some software which relies heavily on Plugins. I plan on writing it in C++ and will most likely be getting plugins built in C++, Python and possibly Java But most likely C++ and python but since I don't want to rule out any languages that may be slightly more obscure or uncommon. **How do I design a plugin that I can use in my C++ program that is unbound to language (or allows for the broadest compatibility)?**

One way that I have thought that this could be done is to offer an API in C++ (as that allows for native code to work) and design a system that will call executable files with parameters being passed as arguments. As long as my program and the Plugin have a specification of what will be passed in and what should happen with the output this should (theoretically) be a valid way of getting maximum compatibility.

The plugins will be responsible for processing data in the form of file(s) and output more files. The actual task that the plugin completes is dependent on the Plugin but the output should always be a file. The plugin will run in parallel to my program with it being called when there is a file to process. There could be multiple instances of a single plugin running at the same time, working of different files. There is no limit to the amount of time that it takes to complete the task (from my programs point of view), but the faster it is done the better. Plugins do not affect the main program directly, only when being given files to work on and collecting results, which is why I think my executable strategy may work for my specific use case.

- [c++](https://softwareengineering.stackexchange.com/questions/tagged/c%2b%2b "show questions tagged 'c++'")
- [language-agnostic](https://softwareengineering.stackexchange.com/questions/tagged/language-agnostic "show questions tagged 'language-agnostic'")
- [desktop-application](https://softwareengineering.stackexchange.com/questions/tagged/desktop-application "show questions tagged 'desktop-application'")
- [plugins](https://softwareengineering.stackexchange.com/questions/tagged/plugins "show questions tagged 'plugins'")
- [plugin-architecture](https://softwareengineering.stackexchange.com/questions/tagged/plugin-architecture "show questions tagged 'plugin-architecture'")

[Share](https://softwareengineering.stackexchange.com/q/358750)

Share a link to this question

Copy link [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/ "The current license for this post: CC BY-SA 3.0")

Short permalink to this question

[Improve this question](https://softwareengineering.stackexchange.com/posts/358750/edit "")

Follow



Follow this question to receive notifications

asked Oct 7, 2017 at 15:49

[![user3797758's user avatar](https://i.sstatic.net/EjWrX.png?s=64)](https://softwareengineering.stackexchange.com/users/284859/user3797758)

[user3797758](https://softwareengineering.stackexchange.com/users/284859/user3797758)

30144 silver badges99 bronze badges

13

- 4





Possible duplicate of [Writing a language agnostic API?](https://softwareengineering.stackexchange.com/questions/358532/writing-a-language-agnostic-api)



amon


–
[amon](https://softwareengineering.stackexchange.com/users/60357/amon "136,209 reputation")



2017-10-07 16:07:40 +00:00

[CommentedOct 7, 2017 at 16:07](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system#comment777130_358750)

- 1





Not really that question is about the API if you read the top second part answer you can see why is this is a different problem to solve



user3797758


–
[user3797758](https://softwareengineering.stackexchange.com/users/284859/user3797758 "301 reputation")



2017-10-07 16:15:57 +00:00

[CommentedOct 7, 2017 at 16:15](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system#comment777131_358750)

- 1





This may be hair-splitting, but this question is about a specific implementation of a solution discussed in the other one. (Not that I'm biased or anything after having written an answer. :-))



Blrfl


–
[Blrfl](https://softwareengineering.stackexchange.com/users/20756/blrfl "20,535 reputation")



2017-10-07 16:38:14 +00:00

[CommentedOct 7, 2017 at 16:38](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system#comment777134_358750)

- 1





I'm pretty sure that your usage of plugin is wrong, and you just want a software pipeline.



Basile Starynkevitch


–
[Basile Starynkevitch](https://softwareengineering.stackexchange.com/users/40065/basile-starynkevitch "32,982 reputation")



2017-10-07 17:32:01 +00:00

[CommentedOct 7, 2017 at 17:32](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system#comment777140_358750)

- 2





It is very sad that most of your questions don't give enough context and motivation. Plese try to improve that in your future questions by telling more about your goals and your work context.



Basile Starynkevitch


–
[Basile Starynkevitch](https://softwareengineering.stackexchange.com/users/40065/basile-starynkevitch "32,982 reputation")



2017-10-08 14:38:47 +00:00

[CommentedOct 8, 2017 at 14:38](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system#comment777248_358750)


[Use comments to ask for more information or suggest improvements. Avoid answering questions in comments.](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system# "Use comments to ask for more information or suggest improvements. Avoid answering questions in comments.") \| [Show **8** more comments](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system# "Expand to show all comments on this post")

## 2 Answers 2

Sorted by:
[Reset to default](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system?answertab=scoredesc#tab-top)

Highest score (default)

Date modified (newest first)

Date created (oldest first)


This answer is useful

14

This answer is not useful

Save this answer.

Loading when this answer was accepted…

[Timeline](https://softwareengineering.stackexchange.com/posts/358754/timeline)

Show activity on this post.

> How do I design a plugin that I can use in my C++ program that is unbound to language (or allows for the broadest compatibility)?

You **cannot do that** for exactly the same reasons explained in [my answer](https://softwareengineering.stackexchange.com/a/358534/40065) to your [previous question](https://softwareengineering.stackexchange.com/q/358532/40065) on _Writing a language agnostic API?_ (which is a duplicate of this one).

You need to **tell more** about your context, **your motivation** your domain. You are seeking for a holy grail design which does not (and probably cannot) exist.

If universal designs existed, you'll already read about them.

Software design is always a matter of trade-off and compromises for a _particular_ problem in a particular domain.

As I told in my other answer, plugins for GCC (a free software compiler, you can study all its source code) are different in their design and goals than plugins for Firefox (a free software web browser).

BTW both [GCC](http://gcc.gnu.org/) and [Clang/LLVM](http://clang.llvm.org/) are free software compilers for C++ accepting plugins, and even if these two software are similar in function, their plugin design is very different and incompatible. The [zsh](https://zsh.org/) and [fish](https://fishshell.com/) shells also accept plugins, and so does [Python](https://python.org/) or [Guile](https://www.gnu.org/software/guile/) or [Lua](https://www.lua.org/) interpreters, or the [emacs](https://www.gnu.org/software/emacs/) or [vim](https://vim.org/) editors.

> The plugins will be responsible for processing data in the form of file(s) and output more files. The actual task that the plugin completes is dependent on the Plugin but the output should always be a file.

There are many languages which have very different views on file IO. Look for example into the IO monad of Haskell (which I don't know well). It is very different of IO in Ocaml or in C++.

> The plugin will run in parallel to my program with it being called when there is a file to process.

## I guess that **your usage of " [plugin](https://en.wikipedia.org/wiki/Plug-in_(computing))" is wrong**.

You should try to find a better word, or give your (unconventional) definition of plugins, or at least illustrate that word by existing examples.

**A plug-in generally involves some [dynamically loaded](https://en.wikipedia.org/wiki/Dynamic_loading)** (using `dlopen` on POSIX, `Load Library` on Windows) **code** (which _extends_ the virtual address space on a process). On Linux, _every program_ written in C++ or C _claiming to accept plugins_ (e.g. `gcc`, `firefox`, `vim`, `gedit`, `vlc`, `xmms2`, Qt and GTK applications, Apache or Lighttpd web servers, ....) _is using [dlopen(3)](http://man7.org/linux/man-pages/man3/dlopen.3.html)_ (perhaps indirectly) _to load the plugin at runtime_, and all the loaded plugins are technically [shared objects](https://en.wikipedia.org/wiki/Library_(computing)#Shared_libraries) in [ELF](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format).

(I don't know Windows, but I guess that all plugins loaded by C++ programs on Windows are [DLL](https://en.wikipedia.org/wiki/Dynamic-link_library) s, and they are loaded with [`LoadLibrary` or similar](https://msdn.microsoft.com/en-us/library/windows/desktop/ms684175(v=vs.85).aspx) function)

* * *

You might not even _need_ any plugin, and you could instead consider [inter-process communication](https://en.wikipedia.org/wiki/Inter-process_communication).

You might be thinking of [**software pipelines**](https://en.wikipedia.org/wiki/Pipeline_(software)) (but they are _not_ plugins). Then think more in terms of [_communication protocols_](https://en.wikipedia.org/wiki/Communications_protocol). Read about [named pipes](https://en.wikipedia.org/wiki/Named_pipe) and [anonymous pipes](https://en.wikipedia.org/wiki/Anonymous_pipe). Look (perhaps for inspiration) into the old [unix pipelines](https://en.wikipedia.org/wiki/Pipeline_(Unix)) (you might want to mimic them on Windows, and some C++ frameworks such as [POCO](http://pocoproject.org/) or [Qt](http://qt.io/) could be helpful). Read about [message passing](https://en.wikipedia.org/wiki/Message_passing).

A software pipeline (and notably [unix pipelines](https://en.wikipedia.org/wiki/Pipeline_(Unix))) enable to make various programs written in _different_ programming languages work together. But you do need to specify some communication protocol. Old Unix utilities supposed a line-by-line protocol, but you might define your messages to be something very different (e.g. JSON, see JSON/RPC for inspiration).

You could also make your C++ application extensible by embedding some interpreter (like [Lua](http://lua.org/), [Guile](https://www.gnu.org/software/guile/), or even [Python](https://docs.python.org/3/extending/embedding.html) ...) or some bytecode VM (like [Parrot](http://parrot.org/), [NekoVM](http://nekovm.org/), ...) in it. But you should _not speak of plugins_ in that case! **You probably should speak of _extensions_, not _plugins_**. For an extremely well known -and decades old- example, [Emacs](https://www.gnu.org/software/emacs/) is an extensible editor (I'm using it daily since the 1990s). You can improve its behavior by customizing it in extension written in [E-Lisp](https://en.wikipedia.org/wiki/Emacs_Lisp). Another example (less familiar to me): [AutoCAD](https://en.wikipedia.org/wiki/AutoCAD) is a proprietary program extensible in [AutoLISP](https://en.wikipedia.org/wiki/AutoLISP); [MicroSoft Word](https://en.wikipedia.org/wiki/Microsoft_Word#Macros) is rumored to be extensible thru macros coded in some [Visual Basic](https://en.wikipedia.org/wiki/Visual_Basic_for_Applications) dialect. A common way to make a software extensible is indeed to embed some interpreter in it. Of course such an approach is _not_ language agnostic, it is tied to the scripting language accepted by that interpreter.

So my concrete recommendation is to **embed some** good enough **interpreter** (like Lua or Guile) in your product, and _accept_ the fact that such extensions won't be language agnostic. In practice, if you choose a good enough extension language, it does not matter much. Both Lua and Guile have been designed as extension languages (and so was Tcl, but I don't recommend using it). My personal preference is Guile (because [Scheme](https://en.wikipedia.org/wiki/Scheme_(programming_language)) is a very elegant and powerful language, quite small and easy to learn, read [SICP](https://mitpress.mit.edu/sicp/)).

> The plugins will be responsible for processing data in the form of file(s) and output more files

If it is only for that (and _nothing more_ ...), you don't need any plugin. You could simply decide that if an output file path name starts with e.g. an exclamation point `!` or a vertical bar `|` your program would pipe to some external command (BTW such conventions are already used in several Unix programs), or have some program argument conventions to behave like that. On POSIX systems, you'll just for example decide to use [popen(3)](http://man7.org/linux/man-pages/man3/popen.3.html) instead of [fopen(3)](http://man7.org/linux/man-pages/man3/fopen.3.html), and that external command could be implemented in any language. You then have a primitive form of software pipelines (which you should of course not call a plugin).

However, I still believe that extending your program by embedding an interpreter is preferable.

[Share](https://softwareengineering.stackexchange.com/a/358754)

Share a link to this answer

Copy link [CC BY-SA 4.0](https://creativecommons.org/licenses/by-sa/4.0/ "The current license for this post: CC BY-SA 4.0")

Short permalink to this answer

[Improve this answer](https://softwareengineering.stackexchange.com/posts/358754/edit "")

Follow



Follow this answer to receive notifications

[edited May 6, 2020 at 15:20](https://softwareengineering.stackexchange.com/posts/358754/revisions "show all edits to this post")

answered Oct 7, 2017 at 17:05

[![Basile Starynkevitch's user avatar](https://i.sstatic.net/Fm52y.png?s=64)](https://softwareengineering.stackexchange.com/users/40065/basile-starynkevitch)

[Basile Starynkevitch](https://softwareengineering.stackexchange.com/users/40065/basile-starynkevitch)

33k66 gold badges9090 silver badges133133 bronze badges

[Add a comment](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system# "Use comments to ask for more information or suggest improvements. Avoid comments like “+1” or “thanks”.") \| [Expand to show all comments on this post](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system# "Expand to show all comments on this post")

This answer is useful

7

This answer is not useful

Save this answer.

Loading when this answer was accepted…

[Timeline](https://softwareengineering.stackexchange.com/posts/358753/timeline)

Show activity on this post.

Process-boundary plugins are a great fit where the compromises you have to make for them (discussed below) are acceptable.

I work on a project with many different types of plugins and for our application, the compromises don't cause any problems. There are many ways to interface with them; the design choice for ours was that the plugin programs communicate with JSON across the standard I/O. Since putting it into production earlier this year, we've been very happy with the decision because it's made the process of expanding our system easier for us as developers and for our users, too.

Pros:

- **Developer Flexibility.** Plugins can be written in any language the author finds comfortable or to be the right tool for the job.
- **Evolvability.** Your system's technology stack can be changed without having to rewrite all of the plugins at the same time.
- **Safety.** If a plugin fails in a catastrophic way, your program is close enough to hear the kaboom but far enough away that it won't be hit by any of the shrapnel.
- **Testability.** The plugin can be tested and tinkered with at the command line without having to link them with your system or into complex test jigs.

Cons, both having to do with overhead:

- **Serialization.** Data you pass to another program has to be in some standard format that probably won't be the same as it exists in memory. This means that there will be time spent encoding and decoding data at both ends of the transaction. You'll also have to pick a format that has libraries available for as wide a variety of languages as plugin authors are likely to use.

- **Startup.** If you're doing high-volume calls to programs that are short-lived and have high startup overhead (Python is the poster child for the latter), you'll find the load on your system goes up considerably. We ran into this on my project and switched one of the plugins from single-use to [streaming](https://www.rfc-editor.org/rfc/rfc7464), where the program is started once and given one blob of work at a time to chew on. (This was accompanied by code to maintain variably-sized pools so programs that went unused didn't linger too long.) For your application, this doesn't sound like a problem, but for others, this plus serialization and communication may add unacceptable latency.


[Share](https://softwareengineering.stackexchange.com/a/358753)

Share a link to this answer

Copy link [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/ "The current license for this post: CC BY-SA 3.0")

Short permalink to this answer

[Improve this answer](https://softwareengineering.stackexchange.com/posts/358753/edit "")

Follow



Follow this answer to receive notifications

[edited Oct 7, 2021 at 7:34](https://softwareengineering.stackexchange.com/posts/358753/revisions "show all edits to this post")

[![Community's user avatar](https://www.gravatar.com/avatar/a007be5a61f6aa8f3e85ae2fc18dd66e?s=64&d=identicon&r=PG)](https://softwareengineering.stackexchange.com/users/-1/community)

[Community](https://softwareengineering.stackexchange.com/users/-1/community) Bot

1

answered Oct 7, 2017 at 16:36

[![Blrfl's user avatar](https://i.sstatic.net/xqNrn.jpg?s=64)](https://softwareengineering.stackexchange.com/users/20756/blrfl)

[Blrfl](https://softwareengineering.stackexchange.com/users/20756/blrfl)

20.5k22 gold badges5454 silver badges7676 bronze badges

4

- I might be misunderstanding the question, answer or both so I could be wrong, but how exactly does this answer the question? Maybe this part: "The programs all communicate with JSON across the standard I/O"? The rest is benefits of having such a system, and the overhead. But the overhead depends on the implementation, which, if I understand correctly, is what OP is asking for right?



NickL


–
[NickL](https://softwareengineering.stackexchange.com/users/266660/nickl "260 reputation")



2017-10-08 00:30:56 +00:00

[CommentedOct 8, 2017 at 0:30](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system#comment777183_358753)

- @NickL I took the question to include the second paragraph and answered it in terms of "that's one way to do it, here's how one project did it and these are the trade-offs." I understand your interpretation even if I don't share it.



Blrfl


–
[Blrfl](https://softwareengineering.stackexchange.com/users/20756/blrfl "20,535 reputation")



2017-10-08 12:35:04 +00:00

[CommentedOct 8, 2017 at 12:35](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system#comment777234_358753)

- Ah now I see, you basically elaborated on some of the pros and cons of the technique OP had in mind. However, most of them really depend on the implementation. For example, you say plugins can be written in any language, but how would you run and communicate with a plugin written in PHP? While I see your point, I was expecting these kinds of details.



NickL


–
[NickL](https://softwareengineering.stackexchange.com/users/266660/nickl "260 reputation")



2017-10-08 17:29:27 +00:00

[CommentedOct 8, 2017 at 17:29](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system#comment777273_358753)

- @NickL [That would be a good question to ask on StackOverflow](https://stackoverflow.com/questions/10262532/running-php-script-from-the-command-line).



Blrfl


–
[Blrfl](https://softwareengineering.stackexchange.com/users/20756/blrfl "20,535 reputation")



2017-10-08 18:01:03 +00:00

[CommentedOct 8, 2017 at 18:01](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system#comment777276_358753)


[Add a comment](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system# "Use comments to ask for more information or suggest improvements. Avoid comments like “+1” or “thanks”.") \| [Expand to show all comments on this post](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system# "Expand to show all comments on this post")

**[Protected question](https://softwareengineering.stackexchange.com/help/privileges/protect-questions)**. To answer this question, you need to have at least 10 reputation on this site (not counting the [association bonus](https://meta.stackexchange.com/questions/141648/what-is-the-association-bonus-and-how-does-it-work)). The reputation requirement helps protect this question from spam and non-answer activity.



Start asking to get answers

Find the answer to your question by asking.

[Ask question](https://softwareengineering.stackexchange.com/questions/ask)

Explore related questions

- [c++](https://softwareengineering.stackexchange.com/questions/tagged/c%2b%2b "show questions tagged 'c++'")
- [language-agnostic](https://softwareengineering.stackexchange.com/questions/tagged/language-agnostic "show questions tagged 'language-agnostic'")
- [desktop-application](https://softwareengineering.stackexchange.com/questions/tagged/desktop-application "show questions tagged 'desktop-application'")
- [plugins](https://softwareengineering.stackexchange.com/questions/tagged/plugins "show questions tagged 'plugins'")
- [plugin-architecture](https://softwareengineering.stackexchange.com/questions/tagged/plugin-architecture "show questions tagged 'plugin-architecture'")

See similar questions with these tags.

- The Overflow Blog

- [Breaking your AI storage bottlenecks](https://stackoverflow.blog/2026/05/22/breaking-your-ai-storage-bottlenecks/?cb=1)

- [Dispatches from O'Reilly: The accidental orchestrator](https://stackoverflow.blog/2026/05/22/dispatches-from-o-reilly-the-accidental-orchestrator/?cb=1)

- Featured on Meta

- [(Almost) One year of Challenges](https://meta.stackexchange.com/questions/418261/almost-one-year-of-challenges?cb=1)


### Linked

[8](https://softwareengineering.stackexchange.com/questions/358532/writing-a-language-agnostic-api?lq=1 "Question score (upvotes - downvotes)") [Writing a language agnostic API?](https://softwareengineering.stackexchange.com/questions/358532/writing-a-language-agnostic-api?noredirect=1&lq=1)

### Related

[3](https://softwareengineering.stackexchange.com/questions/123146/designing-a-plugin-based-architecture-what-is-a-protocol-service-supposed-to-p?rq=1 "Question score (upvotes - downvotes)") [Designing a plugin-based architecture - what is a protocol service supposed to provide to a plugin?](https://softwareengineering.stackexchange.com/questions/123146/designing-a-plugin-based-architecture-what-is-a-protocol-service-supposed-to-p?rq=1)

[7](https://softwareengineering.stackexchange.com/questions/131084/designing-web-based-plugin-systems-correctly-so-they-dont-waste-as-many-resourc?rq=1 "Question score (upvotes - downvotes)") [Designing web-based plugin systems correctly so they don't waste as many resources?](https://softwareengineering.stackexchange.com/questions/131084/designing-web-based-plugin-systems-correctly-so-they-dont-waste-as-many-resourc?rq=1)

[1](https://softwareengineering.stackexchange.com/questions/288206/arbitrary-data-shared-between-plugins?rq=1 "Question score (upvotes - downvotes)") [arbitrary data shared between plugins](https://softwareengineering.stackexchange.com/questions/288206/arbitrary-data-shared-between-plugins?rq=1)

[2](https://softwareengineering.stackexchange.com/questions/330970/what-should-you-replace-an-enumeration-with-if-values-are-to-be-provided-by-plug?rq=1 "Question score (upvotes - downvotes)") [What should you replace an enumeration with if values are to be provided by plugins?](https://softwareengineering.stackexchange.com/questions/330970/what-should-you-replace-an-enumeration-with-if-values-are-to-be-provided-by-plug?rq=1)

[4](https://softwareengineering.stackexchange.com/questions/348239/how-to-share-dependent-classes-between-a-main-app-and-plugins-in-java?rq=1 "Question score (upvotes - downvotes)") [How to share dependent classes between a main app and plugins in Java?](https://softwareengineering.stackexchange.com/questions/348239/how-to-share-dependent-classes-between-a-main-app-and-plugins-in-java?rq=1)

[8](https://softwareengineering.stackexchange.com/questions/358532/writing-a-language-agnostic-api?rq=1 "Question score (upvotes - downvotes)") [Writing a language agnostic API?](https://softwareengineering.stackexchange.com/questions/358532/writing-a-language-agnostic-api?rq=1)

[1](https://softwareengineering.stackexchange.com/questions/447610/safe-plugin-architecture-for-python-web-api?rq=1 "Question score (upvotes - downvotes)") [Safe Plugin Architecture for Python Web API](https://softwareengineering.stackexchange.com/questions/447610/safe-plugin-architecture-for-python-web-api?rq=1)

[0](https://softwareengineering.stackexchange.com/questions/456458/versioning-for-a-set-of-plugin-libraries-that-supports-foundational-concepts-wit?rq=1 "Question score (upvotes - downvotes)") [Versioning for a set of plugin libraries that supports foundational concepts within various toolsets](https://softwareengineering.stackexchange.com/questions/456458/versioning-for-a-set-of-plugin-libraries-that-supports-foundational-concepts-wit?rq=1)

#### [Hot Network Questions](https://stackexchange.com/questions?tab=hot)

- [which way to turn on bathroom faucet with crystal ball handles](https://diy.stackexchange.com/questions/330645/which-way-to-turn-on-bathroom-faucet-with-crystal-ball-handles)
- [Is there an official class that would fit the "blood mage" archetype?](https://rpg.stackexchange.com/questions/219384/is-there-an-official-class-that-would-fit-the-blood-mage-archetype)
- [Similarity between Triple product and baryon singlet color wave function?](https://physics.stackexchange.com/questions/872586/similarity-between-triple-product-and-baryon-singlet-color-wave-function)
- [How do I sync up animated RuleTiles in Unity?](https://gamedev.stackexchange.com/questions/217252/how-do-i-sync-up-animated-ruletiles-in-unity)
- [Why can't TeX \\par remove these spaces?](https://tex.stackexchange.com/questions/763053/why-cant-tex-par-remove-these-spaces)
- [How to replace these 2" swimming pool PVC valves?](https://diy.stackexchange.com/questions/330627/how-to-replace-these-2-swimming-pool-pvc-valves)
- [Why does the definition of spinor require a metric?](https://physics.stackexchange.com/questions/872638/why-does-the-definition-of-spinor-require-a-metric)
- [Early career decision: renewable Assistant Teaching Professor role at reputable R1 or Tenure-Track role at precarious SLAC](https://academia.stackexchange.com/questions/226778/early-career-decision-renewable-assistant-teaching-professor-role-at-reputable)
- [What's the difference between these two accents?](https://music.stackexchange.com/questions/143718/whats-the-difference-between-these-two-accents)
- [Sequence of functions: easing package](https://tex.stackexchange.com/questions/763046/sequence-of-functions-easing-package)
- [How to save face when forced to NOT help?](https://workplace.stackexchange.com/questions/203444/how-to-save-face-when-forced-to-not-help)
- [Can two same/identical survivor functions have different hazard funtions?](https://stats.stackexchange.com/questions/676025/can-two-same-identical-survivor-functions-have-different-hazard-funtions)
- [What can mathematicians do to mitigate the deleterious impacts of AI?](https://mathoverflow.net/questions/511572/what-can-mathematicians-do-to-mitigate-the-deleterious-impacts-of-ai)
- [Can my multimeter accurately measure the RMS voltage of this square signal?](https://electronics.stackexchange.com/questions/769185/can-my-multimeter-accurately-measure-the-rms-voltage-of-this-square-signal)
- [What should one do with small numerical improvements to great results?](https://mathoverflow.net/questions/511579/what-should-one-do-with-small-numerical-improvements-to-great-results)
- [What classic paintings do Mr. 3's wax art pieces reference in One Piece S02E05?](https://movies.stackexchange.com/questions/131867/what-classic-paintings-do-mr-3s-wax-art-pieces-reference-in-one-piece-s02e05)
- [Cycles via Beta reduction](https://cs.stackexchange.com/questions/176399/cycles-via-beta-reduction)
- [Sequence of partially colored circles](https://puzzling.stackexchange.com/questions/138182/sequence-of-partially-colored-circles)
- [Is it safe to freeze leftover chicken that was originally refrigerated vacuum sealed](https://cooking.stackexchange.com/questions/136953/is-it-safe-to-freeze-leftover-chicken-that-was-originally-refrigerated-vacuum-se)
- [Second PhD in the same field instead of postdoc, since there are more PhD positions available](https://academia.stackexchange.com/questions/226790/second-phd-in-the-same-field-instead-of-postdoc-since-there-are-more-phd-positi)
- [How can consuming alcohol amplify fire magic?](https://worldbuilding.stackexchange.com/questions/273496/how-can-consuming-alcohol-amplify-fire-magic)
- [is checking for coagulation time in a microwave an good way to check if milk has gone bad?](https://cooking.stackexchange.com/questions/136959/is-checking-for-coagulation-time-in-a-microwave-an-good-way-to-check-if-milk-has)
- [expression: relinquish a sigh](https://english.stackexchange.com/questions/639803/expression-relinquish-a-sigh)
- [Genetic Algorithm to maximize a function over a constrained search space](https://codereview.stackexchange.com/questions/302217/genetic-algorithm-to-maximize-a-function-over-a-constrained-search-space)

[Question feed](https://softwareengineering.stackexchange.com/feeds/question/358750 "Feed of this question and its answers")

# Subscribe to RSS

Question feed

To subscribe to this RSS feed, copy and paste this URL into your RSS reader.

[Close](https://softwareengineering.stackexchange.com/questions/358750/making-a-language-agnostic-plugin-system#)

lang-cpp

# Why are you flagging this comment?

It contains harassment, bigotry or abuse.
This comment attacks a person or group. Learn more in our [Abusive behavior policy](https://softwareengineering.stackexchange.com/conduct/abusive-behavior).

It's unfriendly or unkind.
This comment is rude or condescending. Learn more in our [Code of Conduct](https://softwareengineering.stackexchange.com/conduct/abusive-behavior).

Not needed.
This comment is not relevant to the post.

```

```

Enter at least 6 characters

Something else.
A problem not listed above. Try to be as specific as possible.

```

```

Enter at least 6 characters

Flag commentCancel

You have 0 flags left today

# ![Illustration of upvote icon after it is clicked](https://softwareengineering.stackexchange.com/Content/Img/modal/img-upvote.png?v=fce73bd9724d)

# Hang on, you can't upvote just yet.

You'll need to complete a few actions and gain 15 reputation points
before being able to upvote. **Upvoting** indicates when questions and answers are useful. [What's reputation and how do I get it?](https://stackoverflow.com/help/whats-reputation)

Instead, you can save this post to reference later.

Save this post for laterNot now

##### [Software Engineering](https://softwareengineering.stackexchange.com/)

- [Tour](https://softwareengineering.stackexchange.com/tour)
- [Help](https://softwareengineering.stackexchange.com/help)
- [Chat](https://chat.stackexchange.com/?tab=site&host=softwareengineering.stackexchange.com)
- [Contact](https://softwareengineering.stackexchange.com/contact)
- [Feedback](https://softwareengineering.meta.stackexchange.com/)

##### [Company](https://stackoverflow.co/)

- [Stack Overflow](https://stackoverflow.com/)
- [Stack Internal](https://stackoverflow.co/internal/)
- [Stack Data Licensing](https://stackoverflow.co/data-licensing/)
- [Stack Ads](https://stackoverflow.co/advertising/)
- [About](https://stackoverflow.co/)
- [Press](https://stackoverflow.co/company/press/)
- [Legal](https://stackoverflow.com/legal)
- [Privacy Policy](https://stackoverflow.com/legal/privacy-policy)
- [Terms of Service](https://stackoverflow.com/legal/terms-of-service/public)
- Your Privacy Choices
- [Cookie Policy](https://policies.stackoverflow.co/stack-overflow/cookie-policy)

##### [Stack Exchange Network](https://stackexchange.com/)

- [Technology](https://stackexchange.com/sites#technology)
- [Culture & recreation](https://stackexchange.com/sites#culturerecreation)
- [Life & arts](https://stackexchange.com/sites#lifearts)
- [Science](https://stackexchange.com/sites#science)
- [Professional](https://stackexchange.com/sites#professional)
- [Business](https://stackexchange.com/sites#business)
- [API](https://api.stackexchange.com/)
- [Data](https://data.stackexchange.com/)

- [Blog](https://stackoverflow.blog/?blb=1)
- [Facebook](https://www.facebook.com/officialstackoverflow/)
- [Twitter](https://twitter.com/stackoverflow)
- [LinkedIn](https://linkedin.com/company/stack-overflow)
- [Instagram](https://www.instagram.com/thestackoverflow)

Site design / logo © 2026 Stack Exchange Inc; user contributions licensed under [CC BY-SA](https://stackoverflow.com/help/licensing). rev 2026.5.18.43150