[Skip to content](https://github.com/libffi/libffi#start-of-content)

You signed in with another tab or window. [Reload](https://github.com/libffi/libffi) to refresh your session.You signed out in another tab or window. [Reload](https://github.com/libffi/libffi) to refresh your session.You switched accounts on another tab or window. [Reload](https://github.com/libffi/libffi) to refresh your session.Dismiss alert

{{ message }}

[libffi](https://github.com/libffi)/ **[libffi](https://github.com/libffi/libffi)** Public

- [Notifications](https://github.com/login?return_to=%2Flibffi%2Flibffi) You must be signed in to change notification settings
- [Fork\\
814](https://github.com/login?return_to=%2Flibffi%2Flibffi)
- [Star\\
4.3k](https://github.com/login?return_to=%2Flibffi%2Flibffi)


master

[**15** Branches](https://github.com/libffi/libffi/branches) [**30** Tags](https://github.com/libffi/libffi/tags)

[Go to Branches page](https://github.com/libffi/libffi/branches)[Go to Tags page](https://github.com/libffi/libffi/tags)

Go to file

Code

Open more actions menu

## Folders and files

| Name | Name | Last commit message | Last commit date |
| --- | --- | --- | --- |
| ## Latest commit<br>[![jakubjelinek](https://avatars.githubusercontent.com/u/9370665?v=4&size=40)](https://github.com/jakubjelinek)[jakubjelinek](https://github.com/libffi/libffi/commits?author=jakubjelinek)<br>[Fix two comment typos. (](https://github.com/libffi/libffi/commit/c5abbdad2f930f806791942776ccd45beeff1613) [#961](https://github.com/libffi/libffi/pull/961) [)](https://github.com/libffi/libffi/commit/c5abbdad2f930f806791942776ccd45beeff1613)<br>failure<br>5 days agoMay 22, 2026<br>[c5abbda](https://github.com/libffi/libffi/commit/c5abbdad2f930f806791942776ccd45beeff1613) · 5 days agoMay 22, 2026<br>## History<br>[1,932 Commits](https://github.com/libffi/libffi/commits/master/) <br>Open commit details<br>[View commit history for this file.](https://github.com/libffi/libffi/commits/master/) 1,932 Commits |
| [.ci](https://github.com/libffi/libffi/tree/master/.ci ".ci") | [.ci](https://github.com/libffi/libffi/tree/master/.ci ".ci") | [ci: update gcc version to 15 in build process](https://github.com/libffi/libffi/commit/d994395ce74026f67b50957e6ce899e8ddbe5028 "ci: update gcc version to 15 in build process") | 11 months agoJun 9, 2025 |
| [.github](https://github.com/libffi/libffi/tree/master/.github ".github") | [.github](https://github.com/libffi/libffi/tree/master/.github ".github") | [Emscripten: Add wasm64 target (](https://github.com/libffi/libffi/commit/20eacb22e9e9cdd6402cb75278dede2b36051e53 "Emscripten: Add wasm64 target (#927)  * src/wasm32: Allow building with Emscripten with 64bit support  MEMORY64 enables 64bit pointers so this commit updates the accessors for the libffi data structures accordingly.  Each JS functions in ffi.c receives pointers as BigInt (i64) values and with casts them to Numer (i53) using bigintToI53Checked. While memory64 supports 64bit addressing, the maximum memory size is currently limited to 16GiB [1]. Therefore, we can assume that the passed pointers are within the Number's range.  [1] https://webassembly.github.io/memory64/js-api/#limits  Signed-off-by: Kohei Tokunaga <ktokunaga.mail@gmail.com>  * Add wasm64 target to the build scripts  This commit adds support for the wasm64 target via the configure script. Emscripten supports two modes of the -sMEMORY64 flag[1] so the script allows users specifying the value through a configuration variable.  Additionally, \"src/wasm32\" directory has been renamed to the more generic \"src/wasm\" because it's now shared between both 32bit and 64bit builds.  [1] https://emscripten.org/docs/tools_reference/settings_reference.html#memory64  Signed-off-by: Kohei Tokunaga <ktokunaga.mail@gmail.com>  * GitHub Actions: Add wasm64 tests  This commit adds a test matrix for wasm32, wasm64 and wasm64 with the -sMEMORY64=2 flag, using the latest version of Emscripten. -Wno-main is added to suppress the following warning in unwindtest.cc and unwindtest_ffi_call.cc.  > FAIL: libffi.closures/unwindtest_ffi_call.cc -W -Wall -O2 (test for excess errors) > Excess errors: > ./libffi.closures/unwindtest_ffi_call.cc:20:5: warning: 'main' should not be 'extern \"C\"' [-Wmain] >    20 | int main (void) >       |     ^ > 1 warning generated.  Signed-off-by: Kohei Tokunaga <ktokunaga.mail@gmail.com>  * testsuite: Fix types of main function  test_libffi.py calls each test's main function without arguments, but some tests define the main function with parameters. This signature mismatch causes a runtime error with the recent version of Emscripten.  This commit resolves this issue by updating the function signatures to match the way they are called.  Signed-off-by: Kohei Tokunaga <ktokunaga.mail@gmail.com>  * README: Add document about WASM64  Signed-off-by: Kohei Tokunaga <ktokunaga.mail@gmail.com>  ---------  Signed-off-by: Kohei Tokunaga <ktokunaga.mail@gmail.com>") [#927](https://github.com/libffi/libffi/pull/927) [)](https://github.com/libffi/libffi/commit/20eacb22e9e9cdd6402cb75278dede2b36051e53 "Emscripten: Add wasm64 target (#927)  * src/wasm32: Allow building with Emscripten with 64bit support  MEMORY64 enables 64bit pointers so this commit updates the accessors for the libffi data structures accordingly.  Each JS functions in ffi.c receives pointers as BigInt (i64) values and with casts them to Numer (i53) using bigintToI53Checked. While memory64 supports 64bit addressing, the maximum memory size is currently limited to 16GiB [1]. Therefore, we can assume that the passed pointers are within the Number's range.  [1] https://webassembly.github.io/memory64/js-api/#limits  Signed-off-by: Kohei Tokunaga <ktokunaga.mail@gmail.com>  * Add wasm64 target to the build scripts  This commit adds support for the wasm64 target via the configure script. Emscripten supports two modes of the -sMEMORY64 flag[1] so the script allows users specifying the value through a configuration variable.  Additionally, \"src/wasm32\" directory has been renamed to the more generic \"src/wasm\" because it's now shared between both 32bit and 64bit builds.  [1] https://emscripten.org/docs/tools_reference/settings_reference.html#memory64  Signed-off-by: Kohei Tokunaga <ktokunaga.mail@gmail.com>  * GitHub Actions: Add wasm64 tests  This commit adds a test matrix for wasm32, wasm64 and wasm64 with the -sMEMORY64=2 flag, using the latest version of Emscripten. -Wno-main is added to suppress the following warning in unwindtest.cc and unwindtest_ffi_call.cc.  > FAIL: libffi.closures/unwindtest_ffi_call.cc -W -Wall -O2 (test for excess errors) > Excess errors: > ./libffi.closures/unwindtest_ffi_call.cc:20:5: warning: 'main' should not be 'extern \"C\"' [-Wmain] >    20 | int main (void) >       |     ^ > 1 warning generated.  Signed-off-by: Kohei Tokunaga <ktokunaga.mail@gmail.com>  * testsuite: Fix types of main function  test_libffi.py calls each test's main function without arguments, but some tests define the main function with parameters. This signature mismatch causes a runtime error with the recent version of Emscripten.  This commit resolves this issue by updating the function signatures to match the way they are called.  Signed-off-by: Kohei Tokunaga <ktokunaga.mail@gmail.com>  * README: Add document about WASM64  Signed-off-by: Kohei Tokunaga <ktokunaga.mail@gmail.com>  ---------  Signed-off-by: Kohei Tokunaga <ktokunaga.mail@gmail.com>") | 10 months agoAug 2, 2025 |
| [doc](https://github.com/libffi/libffi/tree/master/doc "doc") | [doc](https://github.com/libffi/libffi/tree/master/doc "doc") | [feat: Update libffi version to 3.5.2 with wasm64 and DragonFly BSD su…](https://github.com/libffi/libffi/commit/e2eda0cf72a0598b44278cc91860ea402273fa29 "feat: Update libffi version to 3.5.2 with wasm64 and DragonFly BSD support") | 10 months agoAug 2, 2025 |
| [include](https://github.com/libffi/libffi/tree/master/include "include") | [include](https://github.com/libffi/libffi/tree/master/include "include") | [Add conditional target support for \_\_int128 (](https://github.com/libffi/libffi/commit/840add3b6a1cd405d9ba59da7c507f75f71c808f "Add conditional target support for __int128 (#951)  * powerpc: Move ffi_aix_trampoline_struct to ffi_darwin.c  This is not required for the user of the library, and by placing it in the file in which it is used, we can also remove some ifdefs.  * powerpc: Disconnect closure assembly from FFI_TYPE constants  Define a set of PPC_LD_* constants private to the implementation. This allows some duplicate cases to be removed, as well as handling the more complex structure cases in C instead of asm.  Hoist the 'mtlr r0' before the jump table, allowing two useful insns per case instead of one.  * Detect __int128_t and add ffi_type_[us]int128  * testsuite: Add trivial smoke test for int128  * testsuite: Add trivial smoke test for complex int128  * x86: Fix irregularly sized structure passing for win64  Sizes 1, 2, 4, 8 are passed in integer registers; sizes 3, 5, 6, 7 are passed by reference. C.f. the switch just below in which we install the arguments.  * x86: Support FFI_TARGET_HAS_INT128  * aarch64: Support FFI_TARGET_HAS_INT128  * alpha: Support FFI_TARGET_HAS_INT128  * loongarch64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * riscv64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * s390x: Support FFI_TARGET_HAS_INT128") [#951](https://github.com/libffi/libffi/pull/951) [)](https://github.com/libffi/libffi/commit/840add3b6a1cd405d9ba59da7c507f75f71c808f "Add conditional target support for __int128 (#951)  * powerpc: Move ffi_aix_trampoline_struct to ffi_darwin.c  This is not required for the user of the library, and by placing it in the file in which it is used, we can also remove some ifdefs.  * powerpc: Disconnect closure assembly from FFI_TYPE constants  Define a set of PPC_LD_* constants private to the implementation. This allows some duplicate cases to be removed, as well as handling the more complex structure cases in C instead of asm.  Hoist the 'mtlr r0' before the jump table, allowing two useful insns per case instead of one.  * Detect __int128_t and add ffi_type_[us]int128  * testsuite: Add trivial smoke test for int128  * testsuite: Add trivial smoke test for complex int128  * x86: Fix irregularly sized structure passing for win64  Sizes 1, 2, 4, 8 are passed in integer registers; sizes 3, 5, 6, 7 are passed by reference. C.f. the switch just below in which we install the arguments.  * x86: Support FFI_TARGET_HAS_INT128  * aarch64: Support FFI_TARGET_HAS_INT128  * alpha: Support FFI_TARGET_HAS_INT128  * loongarch64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * riscv64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * s390x: Support FFI_TARGET_HAS_INT128") | 2 months agoMar 8, 2026 |
| [libffi.xcodeproj](https://github.com/libffi/libffi/tree/master/libffi.xcodeproj "libffi.xcodeproj") | [libffi.xcodeproj](https://github.com/libffi/libffi/tree/master/libffi.xcodeproj "libffi.xcodeproj") | [Remove 32-bit x86 file references to fix macosx builds](https://github.com/libffi/libffi/commit/f9da75e157ab089363d079a781644c3e6f7db2c3 "Remove 32-bit x86 file references to fix macosx builds") | 7 years agoNov 30, 2019 |
| [m4](https://github.com/libffi/libffi/tree/master/m4 "m4") | [m4](https://github.com/libffi/libffi/tree/master/m4 "m4") | [Make 3.5.0-pre0 release. Build and publish Windows binaries. Clean up…](https://github.com/libffi/libffi/commit/854ce7be85da6ab571b2f73390be20f57c5afc9b "Make 3.5.0-pre0 release. Build and publish Windows binaries. Clean up testing. (#912)  This commit removes many platforms from the testing workflow. They will be added back in future commits.") | last yearJun 2, 2025 |
| [man](https://github.com/libffi/libffi/tree/master/man "man") | [man](https://github.com/libffi/libffi/tree/master/man "man") | [Remove autogenerated files from the repository](https://github.com/libffi/libffi/commit/35634dbceaac0a1544f7385addc01d21ef1ef6a8 "Remove autogenerated files from the repository  Add an autogen.sh to regenerate them.") | 12 years agoMar 16, 2014 |
| [msvc\_build/aarch64](https://github.com/libffi/libffi/tree/master/msvc_build/aarch64 "This path skips through empty directories") | [msvc\_build/aarch64](https://github.com/libffi/libffi/tree/master/msvc_build/aarch64 "This path skips through empty directories") | [Check if FFI\_GO\_CLOSURES is defined (](https://github.com/libffi/libffi/commit/c23e9a1c81a84ea4804d001865845b25ff8d4c8a "Check if FFI_GO_CLOSURES is defined (#796)  This macro is always defined to 1 if defined, or undefined. With `-Wundef` option, checking the value without checking if it is defined causes warnings:  ``` /opt/local/include/ffi.h:477:5: warning: 'FFI_GO_CLOSURES' is not defined, evaluates to 0 [-Wundef] #if FFI_GO_CLOSURES     ^ ```") [#796](https://github.com/libffi/libffi/pull/796) [)](https://github.com/libffi/libffi/commit/c23e9a1c81a84ea4804d001865845b25ff8d4c8a "Check if FFI_GO_CLOSURES is defined (#796)  This macro is always defined to 1 if defined, or undefined. With `-Wundef` option, checking the value without checking if it is defined causes warnings:  ``` /opt/local/include/ffi.h:477:5: warning: 'FFI_GO_CLOSURES' is not defined, evaluates to 0 [-Wundef] #if FFI_GO_CLOSURES     ^ ```") | 3 years agoOct 21, 2023 |
| [src](https://github.com/libffi/libffi/tree/master/src "src") | [src](https://github.com/libffi/libffi/tree/master/src "src") | [Fix two comment typos. (](https://github.com/libffi/libffi/commit/c5abbdad2f930f806791942776ccd45beeff1613 "Fix two comment typos. (#961)") [#961](https://github.com/libffi/libffi/pull/961) [)](https://github.com/libffi/libffi/commit/c5abbdad2f930f806791942776ccd45beeff1613 "Fix two comment typos. (#961)") | 5 days agoMay 22, 2026 |
| [testsuite](https://github.com/libffi/libffi/tree/master/testsuite "testsuite") | [testsuite](https://github.com/libffi/libffi/tree/master/testsuite "testsuite") | [Add conditional target support for \_\_int128 (](https://github.com/libffi/libffi/commit/840add3b6a1cd405d9ba59da7c507f75f71c808f "Add conditional target support for __int128 (#951)  * powerpc: Move ffi_aix_trampoline_struct to ffi_darwin.c  This is not required for the user of the library, and by placing it in the file in which it is used, we can also remove some ifdefs.  * powerpc: Disconnect closure assembly from FFI_TYPE constants  Define a set of PPC_LD_* constants private to the implementation. This allows some duplicate cases to be removed, as well as handling the more complex structure cases in C instead of asm.  Hoist the 'mtlr r0' before the jump table, allowing two useful insns per case instead of one.  * Detect __int128_t and add ffi_type_[us]int128  * testsuite: Add trivial smoke test for int128  * testsuite: Add trivial smoke test for complex int128  * x86: Fix irregularly sized structure passing for win64  Sizes 1, 2, 4, 8 are passed in integer registers; sizes 3, 5, 6, 7 are passed by reference. C.f. the switch just below in which we install the arguments.  * x86: Support FFI_TARGET_HAS_INT128  * aarch64: Support FFI_TARGET_HAS_INT128  * alpha: Support FFI_TARGET_HAS_INT128  * loongarch64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * riscv64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * s390x: Support FFI_TARGET_HAS_INT128") [#951](https://github.com/libffi/libffi/pull/951) [)](https://github.com/libffi/libffi/commit/840add3b6a1cd405d9ba59da7c507f75f71c808f "Add conditional target support for __int128 (#951)  * powerpc: Move ffi_aix_trampoline_struct to ffi_darwin.c  This is not required for the user of the library, and by placing it in the file in which it is used, we can also remove some ifdefs.  * powerpc: Disconnect closure assembly from FFI_TYPE constants  Define a set of PPC_LD_* constants private to the implementation. This allows some duplicate cases to be removed, as well as handling the more complex structure cases in C instead of asm.  Hoist the 'mtlr r0' before the jump table, allowing two useful insns per case instead of one.  * Detect __int128_t and add ffi_type_[us]int128  * testsuite: Add trivial smoke test for int128  * testsuite: Add trivial smoke test for complex int128  * x86: Fix irregularly sized structure passing for win64  Sizes 1, 2, 4, 8 are passed in integer registers; sizes 3, 5, 6, 7 are passed by reference. C.f. the switch just below in which we install the arguments.  * x86: Support FFI_TARGET_HAS_INT128  * aarch64: Support FFI_TARGET_HAS_INT128  * alpha: Support FFI_TARGET_HAS_INT128  * loongarch64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * riscv64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * s390x: Support FFI_TARGET_HAS_INT128") | 2 months agoMar 8, 2026 |
| [.allow-ai-service](https://github.com/libffi/libffi/blob/master/.allow-ai-service ".allow-ai-service") | [.allow-ai-service](https://github.com/libffi/libffi/blob/master/.allow-ai-service ".allow-ai-service") | [`feat(github-actions): add new build workflow for warp`](https://github.com/libffi/libffi/commit/c07c40ee9444e029bc595860a669de32b7af282a "`feat(github-actions): add new build workflow for warp`") | 2 years agoJan 31, 2024 |
| [.gail-labels](https://github.com/libffi/libffi/blob/master/.gail-labels ".gail-labels") | [.gail-labels](https://github.com/libffi/libffi/blob/master/.gail-labels ".gail-labels") | [feat: add QUESTION to .gail-labels](https://github.com/libffi/libffi/commit/d0f831bca60ea74b55a3155b929f195a971a3cac "feat: add QUESTION to .gail-labels") | 11 months agoJun 21, 2025 |
| [.gitattributes](https://github.com/libffi/libffi/blob/master/.gitattributes ".gitattributes") | [.gitattributes](https://github.com/libffi/libffi/blob/master/.gitattributes ".gitattributes") | [Clean up line endings (](https://github.com/libffi/libffi/commit/e5f0eb1552f222eb2630e40ee348b090d56412a3 "Clean up line endings (#509)  The CLRF visual studio files can be kept that way, but recognized as text. The assembly file can be converted to LF.") [#509](https://github.com/libffi/libffi/pull/509) [)](https://github.com/libffi/libffi/commit/e5f0eb1552f222eb2630e40ee348b090d56412a3 "Clean up line endings (#509)  The CLRF visual studio files can be kept that way, but recognized as text. The assembly file can be converted to LF.") | 7 years agoOct 8, 2019 |
| [.gitignore](https://github.com/libffi/libffi/blob/master/.gitignore ".gitignore") | [.gitignore](https://github.com/libffi/libffi/blob/master/.gitignore ".gitignore") | [Add wasm32 emscripten support (](https://github.com/libffi/libffi/commit/2687cfc5329d08d6bd4d397c1ca8eb0d171e22fd "Add wasm32 emscripten support (#763)  * added build script  * Apply libffi-emscripten patch  * Some changes to wasm32/ffi.c  * Remove exit(0); from test suites  * Fix LONGDOUBLE argument type  * Use more macros in ffi.c  * Use switch statements instead of if chains  * Implemented struct args  * Finish struct implementation  * Partially working closures  * Got closures working (most of closures test suite passes)  * Revert changes to test suite  * Update .gitignore  * Apply code formatter  * Use stackSave and stackRestore rather than directly adjusting stack pointer  * Add missing break  * Fix visibility of ffi_closure_alloc and ffi_closure_free  * Fix FFI_TYPE_STRUCT and FFI_TYPE_LONGDOUBLE when WASM_BIGINT is not used sig needs to be vi here for FFI_TYPE_STRUCT and FFI_TYPE_LONGDOUBLE, noticed this while running the test suite without WASM_BIGINT support.  * Always use dynCall rather than direct wasmTable lookup (function pointer cast emulation changes dynCall)  * Prevent closures.c from duplicating symbols  * Try to set up CI  * Add test with bigint  * Make test methods static  * Remove BigInt shorthand because it messes up terser  * Add selenium tests  * Update tests a bit to try to make CI work  * WASM_BIGINT is a linker flag not a compile flag  * Finish getting CI working (#1)  * update gitignore  * Avoid adding \"use strict;\" to generated JS  This should be controlled by -s STRICT_JS in Emscripten.  * Make JavaScript ES5 compliant  * Remove redundant EXPORTED_RUNTIME_METHODS settings  * Fix definition of DEREF_I16  * Avoid marshalling FFI_TYPE_LONGDOUBLE when WASM_BIGINT is not used  * Add missing FFI_TYPE_STRUCT signature  * Improve test scripts  * Remove redundant EXPORTED_RUNTIME_METHODS settings  * Add missing EOL  * Add struct unpacking tests  * Update ci config to try to actually use WASM_BIGINT  * Revert \"Avoid marshalling FFI_TYPE_LONGDOUBLE when WASM_BIGINT is not used\"  This reverts commit 61bd5a3e20891623715604581b6e872ab3dfab80.  * Fix single_entry_structs tests  * Fix return from closure call  * Fix 64 bit return from closures  * only allocate as much space on stack for return pointer as needed  * Revert \"only allocate as much space on stack for return pointer as needed\"  This reverts commit e54a30faea3803e7ac33eed191bde9e573850fc1.  * xfail two tests  * Fix err_bad_abi test  * Remove test logging junk  * Try to set up long double marshalling for closures  * xfail err_bad_abi  * Fix reference errors in previous commit  * Add missing argument pointer assignment  * Fix signature of function pointer in cls_dbls_struct  * Fix longdouble argument  * Try some changes to bigint handling  * Fix BigInt handling  * Fix cls_longdouble test  * Fix long double closure arg with no WASM_BIGINT  * Use EM_JS to factor out js helpers  * Support for varargs closure calls  * Fix varargs calls  * Fix err_bad_abi test  * Fix typo in previous commit  * Add more assertions to closures test suite  * Fix some asserts  * Add assertions to a few more tests  * Fix some tests  * Fix more floating point assertions  * Update more tests  * Var args for ffi_call  * Don't do node tests  * Macro for allocating on stack  * Add some comments, simplify struct handling  * Try again to fix varargs calls, add comments  * Consolidate WASM_BIGINT conditionals into LOAD_U64 and STORE_U64 macros  * A bit of cleanup  * Fix another typo  * Some fixes to the testsuite  * Another testsuite fix  * Fix varags with closures?  * Another attempt at getting closure varargs to work  * sig is initialized later  * Allow libffi.closures tests to be run  * Improve build script  * Remove redundant semicolons  * Fix a few libffi.closures test failures  * Cleanup  * Legacy dynCall API is no longer used  * Fix FFI_TYPE_LONGDOUBLE offset  * xfail 2 tests for WASM  - closure_loc_fn0; not applicable -- codeloc doesn't point to closure. - huge_struct; function signature too long.  * Revert some redundant dg-output/printf statements  Helps Node.  * Revert \"Don't do node tests\"  This reverts commit a341ef4b.  * Fix assertions in cls_24byte  * More tiny formating fixes to test suite  * Revert \"Revert \"Don't do node tests\"\"  This reverts commit 7722e685ea04e2420e042886816d8c4dd31f5dcb.  * Fix 64 bit returns when WASM_BIGINT is absent  * Fix print statement in cls_24byte  * Add CALL_FUNC_PTR macro to allow pyodide to define custom calling behavior to handle fpcast  * Update single_entry_structs tests  * More explanations  * Fix compile error in last commit  * Add more support for pyodide fpcast emulation, update CI to try to test it  * Clone via https  * Fix path to pyodide emsdk_env  * Add asserts to the rest of the test suite  * Fix test compile errors  * Fix some tests  * Fix cls_ulonglong  * Fix alignment of <4 byte args  * fix cls_ulonglong again  * Use snprintf instead of sprintf  * Should assert than strncmp returned 0  * Fix va_struct1 and va_struct3  * Change double and long double tests  These tests are failing because of a strange bug with prinft and doubles, but I am not convinced it necessarily has anything to do with libffi. This version casts the double to int before printing it and avoids the issue  * Enable node tests  * Revert \"Change double and long double tests\"  This reverts commit 8f3ff89c6577dc99564181cd9974f2f1ba21f1e9.  * Fix PYODIDE_FPCAST flag  * add conftest.py back in  * Fix emcc error: setting `EXPORTED_FUNCTIONS` expects `<class 'list'>` but got `<class 'str'>`  See discussion on https://github.com/pyodide/pyodide/pull/1596  * Remove test.html  * Remove duplicate test file  * More changes from upstream  * Fix some whitespace  * Add some basic debug logging statements  * Reapply libffi.exp changes  * Don't build docs (#7)  Works around build issue makeinfo: command not found.  * Update long double alignment  Emscripten 2.0.26 reduces the aligmnet of long double to 8. Quoting from `ChangeLog.md`:  > The alignment of `long double`, which is a 128-bit floating-point > value implemented in software, is reduced from 16 to 8. The lower > alignment allows `max_align_t` to properly match the alignment we > use for malloc, which is 8 (raising malloc's alignment to achieve > correctness the other way would come with a performance regression). > (#10072)  * Update long double alignment  Emscripten 2.0.26 reduces the aligmnet of long double to 8. Quoting from `ChangeLog.md`:  > The alignment of `long double`, which is a 128-bit floating-point > value implemented in software, is reduced from 16 to 8. The lower > alignment allows `max_align_t` to properly match the alignment we > use for malloc, which is 8 (raising malloc's alignment to achieve > correctness the other way would come with a performance regression). > (#10072)  * Improve error handling a bit (#8)  * Fix handling of signed arguments to ffi_call (#11)  * Fix struct argument handling in ffi_call (#10)  * Remove fpcast emulation tests  * Align the stack to MAX_ALIGN before making call (#12)  * Increase MAX_ARGS  * Cleanup (#14)  * Fix Closure compiler error with -sASSERTIONS=1 (#15)  * Remove function pointer cast emulation (#13)  This reverts commit 593b402 and cbc54da, as it's no longer needed after PR pyodide/pyodide#2019.  * Prefer the `__EMSCRIPTEN__` definition over `EMSCRIPTEN` (#18)  \"The preprocessor define EMSCRIPTEN is deprecated. Don't pass it to code in strict mode. Code should use the define __EMSCRIPTEN__ instead.\" https://github.com/emscripten-core/emscripten/blob/84a634167a1cd9e8c47d37a559688153a4ceace6/emcc.py#L887-L890  * Install autoconf 2.71  * Try again with installing autoconf 2.71  * Fix compatibility with Emscripten 3.1.28  * CI: remove use of `EM_CONFIG` env  See commit: https://github.com/emscripten-core/emsdk/commit/3d87d5ea8143b3636f872fb05b896eb4a19a070b  * Fix cls_multi_schar: cast rest_call to signed char  * Remove test xfails (#17)  * Fix long double when used as a varargs argument  * Enable unwindtest and fix it  * Add EM_JS_DEPS  * Also require convertJsFunctionToWasm  * Run tests very very verbose  * Echo the .emscripten file  * Remove --experimental-wasm-bigint insertion  * Build with assertions  * Move verbosity flags back out of LDFLAGS  * Remove debug print statement  * Use up to date pyodide docker image  * Explicitly cast res_call to fix test failure  * Put back name of main function in cls_longdouble_va.c  * Fix alignment  The stack pointer apparently needs to be aligned to 16. There were some terrible subtle bugs caused by not respecting this. stackAlloc knows that the stack should be 16 aligned, so we can use stackAlloc(0) to enforce this. This way if alignment requirements change, as long as Emscripten updates stackAlloc to continue to enforce them we should be okay.  * Fix handling of systems with no Js bigint integration  When we run the node tests we use node v14 tests (since node v14 is vendored with Emscripten). Node v14 has no Js bigint integration unless the --experimental-wasm-bigint flag is passed. So only the node tests really notice if we get this right. Turns out, it didn't work. We can't call a JavaScript function with 64 bit integer arguments without bigint integration.  In ffi_call, we are trying to call a wasm function that takes 64 bit integer arguments. dynCall is designed to do this. We need to go back to tracking the signature when we don't have WASM_BIGINT, and then use dynCall. This works better now that emscripten can dynamically fill in extra dynCall wrappers: https://github.com/emscripten-core/emscripten/pull/17328  On the other hand, for the closures we are not getting a function pointer as a first argument. We need to make our own wasm legalizer adaptor that splits 64 bit integer arguments and then calls the JavaScript trampoline, then the JavaScript trampoline reassembles them, calls the closure, then splits the result (if it's a 64 bit integer) and the adaptor puts it back together.  * Improvements to emscripten test shell scripts (#21)  This fixes the C++ unwinding tests and makes other minor improvements to the Emscripten test shell scripts.  * Rename the test folder and move test files into emscripten test folder  * Use docker image that has autoconf-2.71  * Cleanup  * Pin emscripten 3.1.30  * Fix build.sh path  * Rearrange ci pipeline  * Fix bpo_38748 test  * Cleanup  * Improvements to comments, add static asserts, and update copyright  * Use `*_js` instead of `*_helper` for EM_JS functions (#22)  * Minor code simplification  * Xfail first dejagnu test to work around emscripten cache messages  See https://github.com/emscripten-core/emscripten/issues/18607  * Remove unneeded xfails  * Shorten conftest.py by using pytest-pyodide  * Apply formatters and linters to emscripten directory  * Fix Emscripten xfail hack  * Fix build-tests script  * Patch emscripten to quiet info messages  * Clean up compiler flags in scripts and remove some settings from circleci config  * Rename emscripten quiet script  * Add missing export  * Don't remove go.exp  * Add reference to emscripten logging issue  ---------  Co-authored-by: Kleis Auke Wolthuizen <info@kleisauke.nl> Co-authored-by: Kleis Auke Wolthuizen <github@kleisauke.nl> Co-authored-by: Christian Heimes <christian@python.org>") [#763](https://github.com/libffi/libffi/pull/763) [)](https://github.com/libffi/libffi/commit/2687cfc5329d08d6bd4d397c1ca8eb0d171e22fd "Add wasm32 emscripten support (#763)  * added build script  * Apply libffi-emscripten patch  * Some changes to wasm32/ffi.c  * Remove exit(0); from test suites  * Fix LONGDOUBLE argument type  * Use more macros in ffi.c  * Use switch statements instead of if chains  * Implemented struct args  * Finish struct implementation  * Partially working closures  * Got closures working (most of closures test suite passes)  * Revert changes to test suite  * Update .gitignore  * Apply code formatter  * Use stackSave and stackRestore rather than directly adjusting stack pointer  * Add missing break  * Fix visibility of ffi_closure_alloc and ffi_closure_free  * Fix FFI_TYPE_STRUCT and FFI_TYPE_LONGDOUBLE when WASM_BIGINT is not used sig needs to be vi here for FFI_TYPE_STRUCT and FFI_TYPE_LONGDOUBLE, noticed this while running the test suite without WASM_BIGINT support.  * Always use dynCall rather than direct wasmTable lookup (function pointer cast emulation changes dynCall)  * Prevent closures.c from duplicating symbols  * Try to set up CI  * Add test with bigint  * Make test methods static  * Remove BigInt shorthand because it messes up terser  * Add selenium tests  * Update tests a bit to try to make CI work  * WASM_BIGINT is a linker flag not a compile flag  * Finish getting CI working (#1)  * update gitignore  * Avoid adding \"use strict;\" to generated JS  This should be controlled by -s STRICT_JS in Emscripten.  * Make JavaScript ES5 compliant  * Remove redundant EXPORTED_RUNTIME_METHODS settings  * Fix definition of DEREF_I16  * Avoid marshalling FFI_TYPE_LONGDOUBLE when WASM_BIGINT is not used  * Add missing FFI_TYPE_STRUCT signature  * Improve test scripts  * Remove redundant EXPORTED_RUNTIME_METHODS settings  * Add missing EOL  * Add struct unpacking tests  * Update ci config to try to actually use WASM_BIGINT  * Revert \"Avoid marshalling FFI_TYPE_LONGDOUBLE when WASM_BIGINT is not used\"  This reverts commit 61bd5a3e20891623715604581b6e872ab3dfab80.  * Fix single_entry_structs tests  * Fix return from closure call  * Fix 64 bit return from closures  * only allocate as much space on stack for return pointer as needed  * Revert \"only allocate as much space on stack for return pointer as needed\"  This reverts commit e54a30faea3803e7ac33eed191bde9e573850fc1.  * xfail two tests  * Fix err_bad_abi test  * Remove test logging junk  * Try to set up long double marshalling for closures  * xfail err_bad_abi  * Fix reference errors in previous commit  * Add missing argument pointer assignment  * Fix signature of function pointer in cls_dbls_struct  * Fix longdouble argument  * Try some changes to bigint handling  * Fix BigInt handling  * Fix cls_longdouble test  * Fix long double closure arg with no WASM_BIGINT  * Use EM_JS to factor out js helpers  * Support for varargs closure calls  * Fix varargs calls  * Fix err_bad_abi test  * Fix typo in previous commit  * Add more assertions to closures test suite  * Fix some asserts  * Add assertions to a few more tests  * Fix some tests  * Fix more floating point assertions  * Update more tests  * Var args for ffi_call  * Don't do node tests  * Macro for allocating on stack  * Add some comments, simplify struct handling  * Try again to fix varargs calls, add comments  * Consolidate WASM_BIGINT conditionals into LOAD_U64 and STORE_U64 macros  * A bit of cleanup  * Fix another typo  * Some fixes to the testsuite  * Another testsuite fix  * Fix varags with closures?  * Another attempt at getting closure varargs to work  * sig is initialized later  * Allow libffi.closures tests to be run  * Improve build script  * Remove redundant semicolons  * Fix a few libffi.closures test failures  * Cleanup  * Legacy dynCall API is no longer used  * Fix FFI_TYPE_LONGDOUBLE offset  * xfail 2 tests for WASM  - closure_loc_fn0; not applicable -- codeloc doesn't point to closure. - huge_struct; function signature too long.  * Revert some redundant dg-output/printf statements  Helps Node.  * Revert \"Don't do node tests\"  This reverts commit a341ef4b.  * Fix assertions in cls_24byte  * More tiny formating fixes to test suite  * Revert \"Revert \"Don't do node tests\"\"  This reverts commit 7722e685ea04e2420e042886816d8c4dd31f5dcb.  * Fix 64 bit returns when WASM_BIGINT is absent  * Fix print statement in cls_24byte  * Add CALL_FUNC_PTR macro to allow pyodide to define custom calling behavior to handle fpcast  * Update single_entry_structs tests  * More explanations  * Fix compile error in last commit  * Add more support for pyodide fpcast emulation, update CI to try to test it  * Clone via https  * Fix path to pyodide emsdk_env  * Add asserts to the rest of the test suite  * Fix test compile errors  * Fix some tests  * Fix cls_ulonglong  * Fix alignment of <4 byte args  * fix cls_ulonglong again  * Use snprintf instead of sprintf  * Should assert than strncmp returned 0  * Fix va_struct1 and va_struct3  * Change double and long double tests  These tests are failing because of a strange bug with prinft and doubles, but I am not convinced it necessarily has anything to do with libffi. This version casts the double to int before printing it and avoids the issue  * Enable node tests  * Revert \"Change double and long double tests\"  This reverts commit 8f3ff89c6577dc99564181cd9974f2f1ba21f1e9.  * Fix PYODIDE_FPCAST flag  * add conftest.py back in  * Fix emcc error: setting `EXPORTED_FUNCTIONS` expects `<class 'list'>` but got `<class 'str'>`  See discussion on https://github.com/pyodide/pyodide/pull/1596  * Remove test.html  * Remove duplicate test file  * More changes from upstream  * Fix some whitespace  * Add some basic debug logging statements  * Reapply libffi.exp changes  * Don't build docs (#7)  Works around build issue makeinfo: command not found.  * Update long double alignment  Emscripten 2.0.26 reduces the aligmnet of long double to 8. Quoting from `ChangeLog.md`:  > The alignment of `long double`, which is a 128-bit floating-point > value implemented in software, is reduced from 16 to 8. The lower > alignment allows `max_align_t` to properly match the alignment we > use for malloc, which is 8 (raising malloc's alignment to achieve > correctness the other way would come with a performance regression). > (#10072)  * Update long double alignment  Emscripten 2.0.26 reduces the aligmnet of long double to 8. Quoting from `ChangeLog.md`:  > The alignment of `long double`, which is a 128-bit floating-point > value implemented in software, is reduced from 16 to 8. The lower > alignment allows `max_align_t` to properly match the alignment we > use for malloc, which is 8 (raising malloc's alignment to achieve > correctness the other way would come with a performance regression). > (#10072)  * Improve error handling a bit (#8)  * Fix handling of signed arguments to ffi_call (#11)  * Fix struct argument handling in ffi_call (#10)  * Remove fpcast emulation tests  * Align the stack to MAX_ALIGN before making call (#12)  * Increase MAX_ARGS  * Cleanup (#14)  * Fix Closure compiler error with -sASSERTIONS=1 (#15)  * Remove function pointer cast emulation (#13)  This reverts commit 593b402 and cbc54da, as it's no longer needed after PR pyodide/pyodide#2019.  * Prefer the `__EMSCRIPTEN__` definition over `EMSCRIPTEN` (#18)  \"The preprocessor define EMSCRIPTEN is deprecated. Don't pass it to code in strict mode. Code should use the define __EMSCRIPTEN__ instead.\" https://github.com/emscripten-core/emscripten/blob/84a634167a1cd9e8c47d37a559688153a4ceace6/emcc.py#L887-L890  * Install autoconf 2.71  * Try again with installing autoconf 2.71  * Fix compatibility with Emscripten 3.1.28  * CI: remove use of `EM_CONFIG` env  See commit: https://github.com/emscripten-core/emsdk/commit/3d87d5ea8143b3636f872fb05b896eb4a19a070b  * Fix cls_multi_schar: cast rest_call to signed char  * Remove test xfails (#17)  * Fix long double when used as a varargs argument  * Enable unwindtest and fix it  * Add EM_JS_DEPS  * Also require convertJsFunctionToWasm  * Run tests very very verbose  * Echo the .emscripten file  * Remove --experimental-wasm-bigint insertion  * Build with assertions  * Move verbosity flags back out of LDFLAGS  * Remove debug print statement  * Use up to date pyodide docker image  * Explicitly cast res_call to fix test failure  * Put back name of main function in cls_longdouble_va.c  * Fix alignment  The stack pointer apparently needs to be aligned to 16. There were some terrible subtle bugs caused by not respecting this. stackAlloc knows that the stack should be 16 aligned, so we can use stackAlloc(0) to enforce this. This way if alignment requirements change, as long as Emscripten updates stackAlloc to continue to enforce them we should be okay.  * Fix handling of systems with no Js bigint integration  When we run the node tests we use node v14 tests (since node v14 is vendored with Emscripten). Node v14 has no Js bigint integration unless the --experimental-wasm-bigint flag is passed. So only the node tests really notice if we get this right. Turns out, it didn't work. We can't call a JavaScript function with 64 bit integer arguments without bigint integration.  In ffi_call, we are trying to call a wasm function that takes 64 bit integer arguments. dynCall is designed to do this. We need to go back to tracking the signature when we don't have WASM_BIGINT, and then use dynCall. This works better now that emscripten can dynamically fill in extra dynCall wrappers: https://github.com/emscripten-core/emscripten/pull/17328  On the other hand, for the closures we are not getting a function pointer as a first argument. We need to make our own wasm legalizer adaptor that splits 64 bit integer arguments and then calls the JavaScript trampoline, then the JavaScript trampoline reassembles them, calls the closure, then splits the result (if it's a 64 bit integer) and the adaptor puts it back together.  * Improvements to emscripten test shell scripts (#21)  This fixes the C++ unwinding tests and makes other minor improvements to the Emscripten test shell scripts.  * Rename the test folder and move test files into emscripten test folder  * Use docker image that has autoconf-2.71  * Cleanup  * Pin emscripten 3.1.30  * Fix build.sh path  * Rearrange ci pipeline  * Fix bpo_38748 test  * Cleanup  * Improvements to comments, add static asserts, and update copyright  * Use `*_js` instead of `*_helper` for EM_JS functions (#22)  * Minor code simplification  * Xfail first dejagnu test to work around emscripten cache messages  See https://github.com/emscripten-core/emscripten/issues/18607  * Remove unneeded xfails  * Shorten conftest.py by using pytest-pyodide  * Apply formatters and linters to emscripten directory  * Fix Emscripten xfail hack  * Fix build-tests script  * Patch emscripten to quiet info messages  * Clean up compiler flags in scripts and remove some settings from circleci config  * Rename emscripten quiet script  * Add missing export  * Don't remove go.exp  * Add reference to emscripten logging issue  ---------  Co-authored-by: Kleis Auke Wolthuizen <info@kleisauke.nl> Co-authored-by: Kleis Auke Wolthuizen <github@kleisauke.nl> Co-authored-by: Christian Heimes <christian@python.org>") | 3 years agoFeb 2, 2023 |
| [ChangeLog.old](https://github.com/libffi/libffi/blob/master/ChangeLog.old "ChangeLog.old") | [ChangeLog.old](https://github.com/libffi/libffi/blob/master/ChangeLog.old "ChangeLog.old") | [Consolidate all of the old ChangeLog files into ChangeLog.old.](https://github.com/libffi/libffi/commit/28a7cc464c21b4955fba28cc55a6f095ddf5838b "Consolidate all of the old ChangeLog files into ChangeLog.old.") | 7 years agoNov 15, 2019 |
| [LICENSE](https://github.com/libffi/libffi/blob/master/LICENSE "LICENSE") | [LICENSE](https://github.com/libffi/libffi/blob/master/LICENSE "LICENSE") | [Update License date and improve rcedit DLL metadata (](https://github.com/libffi/libffi/commit/8b0eab28cb45c6862b30f5cadfc1a7fe996b072c "Update License date and improve rcedit DLL metadata (#919)  * rcedit add architecture and original filename  * Update LICENSE to 2025") [#919](https://github.com/libffi/libffi/pull/919) [)](https://github.com/libffi/libffi/commit/8b0eab28cb45c6862b30f5cadfc1a7fe996b072c "Update License date and improve rcedit DLL metadata (#919)  * rcedit add architecture and original filename  * Update LICENSE to 2025") | 11 months agoJun 8, 2025 |
| [LICENSE-BUILDTOOLS](https://github.com/libffi/libffi/blob/master/LICENSE-BUILDTOOLS "LICENSE-BUILDTOOLS") | [LICENSE-BUILDTOOLS](https://github.com/libffi/libffi/blob/master/LICENSE-BUILDTOOLS "LICENSE-BUILDTOOLS") | [Add missing build script, make\_sunver.pl.](https://github.com/libffi/libffi/commit/ca112537df7b9cdbccad7541aa3cb43b2a2dac9a "Add missing build script, make_sunver.pl.") | 7 years agoOct 26, 2019 |
| [Makefile.am](https://github.com/libffi/libffi/blob/master/Makefile.am "Makefile.am") | [Makefile.am](https://github.com/libffi/libffi/blob/master/Makefile.am "Makefile.am") | [Add support for LoongArch32 (](https://github.com/libffi/libffi/commit/70e6c2615d7deb0529568b013b1c274fc5e24375 "Add support for LoongArch32 (#957)  * Add support for LoongArch32  Change src/loongarch64 to src/loongarch.  Change __loongarch64 to __loongarch_grlen. __loongarch64 is uesd for compatibility with legacy code, new programs should not assume existence of this macro.[1] And there is no __loongarch32 macro for LoongArch32.  Add ilp32s, ilp32f, ilp32d abi. Add ADDI macro.  [1] https://github.com/loongson/la-toolchain-conventions?tab=readme-ov-file#cc-preprocessor-built-in-macro-definitions  * LoongArch: Fix libffi.closures/cls_longdouble.c  CHECK(a8 == 8) fail on LoongArch32.  long double passed by reference on LoongArch32 ilp32 ABI. If a argument register is available, the address is passed in the argument register; otherwise, it is passed on the stack[1].  The address of return value and a1-a7 arguments of cls_ldouble_fn passed by a0-a7 argument registers. The address of a8 argument need to be passed by stack.  But ffi_call_int only allocates cif->bytes(sizeof(long double)*8) bytes space for stack. Both the address of a8 argument and a8 argument are saved at sp+0 address. The lowest 4-byte of a8 argument are overwritten by the address of a8 argument.  Allocates extra conservative estimate space like RISC-V.  [1] https://github.com/loongson/la-abi-specs/blob/release/lapcs.adoc#passing-arguments") [#957](https://github.com/libffi/libffi/pull/957) [)](https://github.com/libffi/libffi/commit/70e6c2615d7deb0529568b013b1c274fc5e24375 "Add support for LoongArch32 (#957)  * Add support for LoongArch32  Change src/loongarch64 to src/loongarch.  Change __loongarch64 to __loongarch_grlen. __loongarch64 is uesd for compatibility with legacy code, new programs should not assume existence of this macro.[1] And there is no __loongarch32 macro for LoongArch32.  Add ilp32s, ilp32f, ilp32d abi. Add ADDI macro.  [1] https://github.com/loongson/la-toolchain-conventions?tab=readme-ov-file#cc-preprocessor-built-in-macro-definitions  * LoongArch: Fix libffi.closures/cls_longdouble.c  CHECK(a8 == 8) fail on LoongArch32.  long double passed by reference on LoongArch32 ilp32 ABI. If a argument register is available, the address is passed in the argument register; otherwise, it is passed on the stack[1].  The address of return value and a1-a7 arguments of cls_ldouble_fn passed by a0-a7 argument registers. The address of a8 argument need to be passed by stack.  But ffi_call_int only allocates cif->bytes(sizeof(long double)*8) bytes space for stack. Both the address of a8 argument and a8 argument are saved at sp+0 address. The lowest 4-byte of a8 argument are overwritten by the address of a8 argument.  Allocates extra conservative estimate space like RISC-V.  [1] https://github.com/loongson/la-abi-specs/blob/release/lapcs.adoc#passing-arguments") | last monthApr 21, 2026 |
| [README.md](https://github.com/libffi/libffi/blob/master/README.md "README.md") | [README.md](https://github.com/libffi/libffi/blob/master/README.md "README.md") | [Remove nios ii credit (port was removed in 3.4.7)](https://github.com/libffi/libffi/commit/9760868682cc9a33008761f158d86481d56738aa "Remove nios ii credit (port was removed in 3.4.7)  Co-Authored-By: Claude Opus 4.6 (1M context) <noreply@anthropic.com>") | last monthApr 21, 2026 |
| [acinclude.m4](https://github.com/libffi/libffi/blob/master/acinclude.m4 "acinclude.m4") | [acinclude.m4](https://github.com/libffi/libffi/blob/master/acinclude.m4 "acinclude.m4") | [Add mold linker to linker checks. (](https://github.com/libffi/libffi/commit/593cb01a46e63d5361f2df803947cb657bb4e822 "Add mold linker to linker checks. (#866)  Signed-off-by: Satadru Pramanik <satadru@gmail.com>") [#866](https://github.com/libffi/libffi/pull/866) [)](https://github.com/libffi/libffi/commit/593cb01a46e63d5361f2df803947cb657bb4e822 "Add mold linker to linker checks. (#866)  Signed-off-by: Satadru Pramanik <satadru@gmail.com>") | 2 years agoDec 13, 2024 |
| [autogen.sh](https://github.com/libffi/libffi/blob/master/autogen.sh "autogen.sh") | [autogen.sh](https://github.com/libffi/libffi/blob/master/autogen.sh "autogen.sh") | [Support -ios triple](https://github.com/libffi/libffi/commit/19ab448d84223d2992048bce4e926eac2c44f606 "Support -ios triple  Autoconf hasn’t had an update since 2014, and it doesn’t look like it will soon[1] This updates config.{guess,sub}  It adds support for e.g. `-ios`, which allows to have targets like `aarch64-apple-ios`.  It basically does exactly what the config.guess script says: > It is advised that you download the most up to date version of the config scripts from  The configure.ac script has been updated to relax `*-apple-darwin*` to `*-apple-*`. Similarly the `.gitignore` and `autogen.sh` needed to be updated to respect the newer `config.{sub,guess}`  — [1]: http://lists.gnu.org/archive/html/autoconf/2016-07/msg00017.html") | 9 years agoMay 15, 2017 |
| [config.guess](https://github.com/libffi/libffi/blob/master/config.guess "config.guess") | [config.guess](https://github.com/libffi/libffi/blob/master/config.guess "config.guess") | [Import from upstream](https://github.com/libffi/libffi/commit/6993bc14dad1cd24294d64bf91e4503a4d7835d6 "Import from upstream") | 2 years agoJun 1, 2024 |
| [config.sub](https://github.com/libffi/libffi/blob/master/config.sub "config.sub") | [config.sub](https://github.com/libffi/libffi/blob/master/config.sub "config.sub") | [Fix config.sub on Apple platforms (](https://github.com/libffi/libffi/commit/d77b9fefa25f7f11dcc4e6380c661c4cf2960a85 "Fix config.sub on Apple platforms (#860)  * update config.sub  * update config.sub") [#860](https://github.com/libffi/libffi/pull/860) [)](https://github.com/libffi/libffi/commit/d77b9fefa25f7f11dcc4e6380c661c4cf2960a85 "Fix config.sub on Apple platforms (#860)  * update config.sub  * update config.sub") | 2 years agoDec 13, 2024 |
| [configure.ac](https://github.com/libffi/libffi/blob/master/configure.ac "configure.ac") | [configure.ac](https://github.com/libffi/libffi/blob/master/configure.ac "configure.ac") | [Add support for LoongArch32 (](https://github.com/libffi/libffi/commit/70e6c2615d7deb0529568b013b1c274fc5e24375 "Add support for LoongArch32 (#957)  * Add support for LoongArch32  Change src/loongarch64 to src/loongarch.  Change __loongarch64 to __loongarch_grlen. __loongarch64 is uesd for compatibility with legacy code, new programs should not assume existence of this macro.[1] And there is no __loongarch32 macro for LoongArch32.  Add ilp32s, ilp32f, ilp32d abi. Add ADDI macro.  [1] https://github.com/loongson/la-toolchain-conventions?tab=readme-ov-file#cc-preprocessor-built-in-macro-definitions  * LoongArch: Fix libffi.closures/cls_longdouble.c  CHECK(a8 == 8) fail on LoongArch32.  long double passed by reference on LoongArch32 ilp32 ABI. If a argument register is available, the address is passed in the argument register; otherwise, it is passed on the stack[1].  The address of return value and a1-a7 arguments of cls_ldouble_fn passed by a0-a7 argument registers. The address of a8 argument need to be passed by stack.  But ffi_call_int only allocates cif->bytes(sizeof(long double)*8) bytes space for stack. Both the address of a8 argument and a8 argument are saved at sp+0 address. The lowest 4-byte of a8 argument are overwritten by the address of a8 argument.  Allocates extra conservative estimate space like RISC-V.  [1] https://github.com/loongson/la-abi-specs/blob/release/lapcs.adoc#passing-arguments") [#957](https://github.com/libffi/libffi/pull/957) [)](https://github.com/libffi/libffi/commit/70e6c2615d7deb0529568b013b1c274fc5e24375 "Add support for LoongArch32 (#957)  * Add support for LoongArch32  Change src/loongarch64 to src/loongarch.  Change __loongarch64 to __loongarch_grlen. __loongarch64 is uesd for compatibility with legacy code, new programs should not assume existence of this macro.[1] And there is no __loongarch32 macro for LoongArch32.  Add ilp32s, ilp32f, ilp32d abi. Add ADDI macro.  [1] https://github.com/loongson/la-toolchain-conventions?tab=readme-ov-file#cc-preprocessor-built-in-macro-definitions  * LoongArch: Fix libffi.closures/cls_longdouble.c  CHECK(a8 == 8) fail on LoongArch32.  long double passed by reference on LoongArch32 ilp32 ABI. If a argument register is available, the address is passed in the argument register; otherwise, it is passed on the stack[1].  The address of return value and a1-a7 arguments of cls_ldouble_fn passed by a0-a7 argument registers. The address of a8 argument need to be passed by stack.  But ffi_call_int only allocates cif->bytes(sizeof(long double)*8) bytes space for stack. Both the address of a8 argument and a8 argument are saved at sp+0 address. The lowest 4-byte of a8 argument are overwritten by the address of a8 argument.  Allocates extra conservative estimate space like RISC-V.  [1] https://github.com/loongson/la-abi-specs/blob/release/lapcs.adoc#passing-arguments") | last monthApr 21, 2026 |
| [configure.host](https://github.com/libffi/libffi/blob/master/configure.host "configure.host") | [configure.host](https://github.com/libffi/libffi/blob/master/configure.host "configure.host") | [Add support for LoongArch32 (](https://github.com/libffi/libffi/commit/70e6c2615d7deb0529568b013b1c274fc5e24375 "Add support for LoongArch32 (#957)  * Add support for LoongArch32  Change src/loongarch64 to src/loongarch.  Change __loongarch64 to __loongarch_grlen. __loongarch64 is uesd for compatibility with legacy code, new programs should not assume existence of this macro.[1] And there is no __loongarch32 macro for LoongArch32.  Add ilp32s, ilp32f, ilp32d abi. Add ADDI macro.  [1] https://github.com/loongson/la-toolchain-conventions?tab=readme-ov-file#cc-preprocessor-built-in-macro-definitions  * LoongArch: Fix libffi.closures/cls_longdouble.c  CHECK(a8 == 8) fail on LoongArch32.  long double passed by reference on LoongArch32 ilp32 ABI. If a argument register is available, the address is passed in the argument register; otherwise, it is passed on the stack[1].  The address of return value and a1-a7 arguments of cls_ldouble_fn passed by a0-a7 argument registers. The address of a8 argument need to be passed by stack.  But ffi_call_int only allocates cif->bytes(sizeof(long double)*8) bytes space for stack. Both the address of a8 argument and a8 argument are saved at sp+0 address. The lowest 4-byte of a8 argument are overwritten by the address of a8 argument.  Allocates extra conservative estimate space like RISC-V.  [1] https://github.com/loongson/la-abi-specs/blob/release/lapcs.adoc#passing-arguments") [#957](https://github.com/libffi/libffi/pull/957) [)](https://github.com/libffi/libffi/commit/70e6c2615d7deb0529568b013b1c274fc5e24375 "Add support for LoongArch32 (#957)  * Add support for LoongArch32  Change src/loongarch64 to src/loongarch.  Change __loongarch64 to __loongarch_grlen. __loongarch64 is uesd for compatibility with legacy code, new programs should not assume existence of this macro.[1] And there is no __loongarch32 macro for LoongArch32.  Add ilp32s, ilp32f, ilp32d abi. Add ADDI macro.  [1] https://github.com/loongson/la-toolchain-conventions?tab=readme-ov-file#cc-preprocessor-built-in-macro-definitions  * LoongArch: Fix libffi.closures/cls_longdouble.c  CHECK(a8 == 8) fail on LoongArch32.  long double passed by reference on LoongArch32 ilp32 ABI. If a argument register is available, the address is passed in the argument register; otherwise, it is passed on the stack[1].  The address of return value and a1-a7 arguments of cls_ldouble_fn passed by a0-a7 argument registers. The address of a8 argument need to be passed by stack.  But ffi_call_int only allocates cif->bytes(sizeof(long double)*8) bytes space for stack. Both the address of a8 argument and a8 argument are saved at sp+0 address. The lowest 4-byte of a8 argument are overwritten by the address of a8 argument.  Allocates extra conservative estimate space like RISC-V.  [1] https://github.com/loongson/la-abi-specs/blob/release/lapcs.adoc#passing-arguments") | last monthApr 21, 2026 |
| [generate-darwin-source-and-headers.py](https://github.com/libffi/libffi/blob/master/generate-darwin-source-and-headers.py "generate-darwin-source-and-headers.py") | [generate-darwin-source-and-headers.py](https://github.com/libffi/libffi/blob/master/generate-darwin-source-and-headers.py "generate-darwin-source-and-headers.py") | [Update generate-darwin-source-and-headers.py (](https://github.com/libffi/libffi/commit/2d8868ace7419b377587adf6b63fb276411931dd "Update generate-darwin-source-and-headers.py (#914)") [#914](https://github.com/libffi/libffi/pull/914) [)](https://github.com/libffi/libffi/commit/2d8868ace7419b377587adf6b63fb276411931dd "Update generate-darwin-source-and-headers.py (#914)") | last yearJun 4, 2025 |
| [libffi.map.in](https://github.com/libffi/libffi/blob/master/libffi.map.in "libffi.map.in") | [libffi.map.in](https://github.com/libffi/libffi/blob/master/libffi.map.in "libffi.map.in") | [Add conditional target support for \_\_int128 (](https://github.com/libffi/libffi/commit/840add3b6a1cd405d9ba59da7c507f75f71c808f "Add conditional target support for __int128 (#951)  * powerpc: Move ffi_aix_trampoline_struct to ffi_darwin.c  This is not required for the user of the library, and by placing it in the file in which it is used, we can also remove some ifdefs.  * powerpc: Disconnect closure assembly from FFI_TYPE constants  Define a set of PPC_LD_* constants private to the implementation. This allows some duplicate cases to be removed, as well as handling the more complex structure cases in C instead of asm.  Hoist the 'mtlr r0' before the jump table, allowing two useful insns per case instead of one.  * Detect __int128_t and add ffi_type_[us]int128  * testsuite: Add trivial smoke test for int128  * testsuite: Add trivial smoke test for complex int128  * x86: Fix irregularly sized structure passing for win64  Sizes 1, 2, 4, 8 are passed in integer registers; sizes 3, 5, 6, 7 are passed by reference. C.f. the switch just below in which we install the arguments.  * x86: Support FFI_TARGET_HAS_INT128  * aarch64: Support FFI_TARGET_HAS_INT128  * alpha: Support FFI_TARGET_HAS_INT128  * loongarch64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * riscv64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * s390x: Support FFI_TARGET_HAS_INT128") [#951](https://github.com/libffi/libffi/pull/951) [)](https://github.com/libffi/libffi/commit/840add3b6a1cd405d9ba59da7c507f75f71c808f "Add conditional target support for __int128 (#951)  * powerpc: Move ffi_aix_trampoline_struct to ffi_darwin.c  This is not required for the user of the library, and by placing it in the file in which it is used, we can also remove some ifdefs.  * powerpc: Disconnect closure assembly from FFI_TYPE constants  Define a set of PPC_LD_* constants private to the implementation. This allows some duplicate cases to be removed, as well as handling the more complex structure cases in C instead of asm.  Hoist the 'mtlr r0' before the jump table, allowing two useful insns per case instead of one.  * Detect __int128_t and add ffi_type_[us]int128  * testsuite: Add trivial smoke test for int128  * testsuite: Add trivial smoke test for complex int128  * x86: Fix irregularly sized structure passing for win64  Sizes 1, 2, 4, 8 are passed in integer registers; sizes 3, 5, 6, 7 are passed by reference. C.f. the switch just below in which we install the arguments.  * x86: Support FFI_TARGET_HAS_INT128  * aarch64: Support FFI_TARGET_HAS_INT128  * alpha: Support FFI_TARGET_HAS_INT128  * loongarch64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * riscv64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * s390x: Support FFI_TARGET_HAS_INT128") | 2 months agoMar 8, 2026 |
| [libffi.pc.in](https://github.com/libffi/libffi/blob/master/libffi.pc.in "libffi.pc.in") | [libffi.pc.in](https://github.com/libffi/libffi/blob/master/libffi.pc.in "libffi.pc.in") | [Install public headers in the standard path](https://github.com/libffi/libffi/commit/982b89c01aca99c7bc229914fc1521f96930919b "Install public headers in the standard path") | 10 years agoNov 13, 2016 |
| [libtool-ldflags](https://github.com/libffi/libffi/blob/master/libtool-ldflags "libtool-ldflags") | [libtool-ldflags](https://github.com/libffi/libffi/blob/master/libtool-ldflags "libtool-ldflags") | [Re-add libtool-ldflags](https://github.com/libffi/libffi/commit/2f44952c95765c1486fad66f57235f8d459a9748 "Re-add libtool-ldflags") | 12 years agoMar 16, 2014 |
| [libtool-version](https://github.com/libffi/libffi/blob/master/libtool-version "libtool-version") | [libtool-version](https://github.com/libffi/libffi/blob/master/libtool-version "libtool-version") | [Add conditional target support for \_\_int128 (](https://github.com/libffi/libffi/commit/840add3b6a1cd405d9ba59da7c507f75f71c808f "Add conditional target support for __int128 (#951)  * powerpc: Move ffi_aix_trampoline_struct to ffi_darwin.c  This is not required for the user of the library, and by placing it in the file in which it is used, we can also remove some ifdefs.  * powerpc: Disconnect closure assembly from FFI_TYPE constants  Define a set of PPC_LD_* constants private to the implementation. This allows some duplicate cases to be removed, as well as handling the more complex structure cases in C instead of asm.  Hoist the 'mtlr r0' before the jump table, allowing two useful insns per case instead of one.  * Detect __int128_t and add ffi_type_[us]int128  * testsuite: Add trivial smoke test for int128  * testsuite: Add trivial smoke test for complex int128  * x86: Fix irregularly sized structure passing for win64  Sizes 1, 2, 4, 8 are passed in integer registers; sizes 3, 5, 6, 7 are passed by reference. C.f. the switch just below in which we install the arguments.  * x86: Support FFI_TARGET_HAS_INT128  * aarch64: Support FFI_TARGET_HAS_INT128  * alpha: Support FFI_TARGET_HAS_INT128  * loongarch64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * riscv64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * s390x: Support FFI_TARGET_HAS_INT128") [#951](https://github.com/libffi/libffi/pull/951) [)](https://github.com/libffi/libffi/commit/840add3b6a1cd405d9ba59da7c507f75f71c808f "Add conditional target support for __int128 (#951)  * powerpc: Move ffi_aix_trampoline_struct to ffi_darwin.c  This is not required for the user of the library, and by placing it in the file in which it is used, we can also remove some ifdefs.  * powerpc: Disconnect closure assembly from FFI_TYPE constants  Define a set of PPC_LD_* constants private to the implementation. This allows some duplicate cases to be removed, as well as handling the more complex structure cases in C instead of asm.  Hoist the 'mtlr r0' before the jump table, allowing two useful insns per case instead of one.  * Detect __int128_t and add ffi_type_[us]int128  * testsuite: Add trivial smoke test for int128  * testsuite: Add trivial smoke test for complex int128  * x86: Fix irregularly sized structure passing for win64  Sizes 1, 2, 4, 8 are passed in integer registers; sizes 3, 5, 6, 7 are passed by reference. C.f. the switch just below in which we install the arguments.  * x86: Support FFI_TARGET_HAS_INT128  * aarch64: Support FFI_TARGET_HAS_INT128  * alpha: Support FFI_TARGET_HAS_INT128  * loongarch64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * riscv64: Support FFI_TARGET_HAS_INT128  The marshal and unmarshal routines already correctly handle __int128 based on size.  * s390x: Support FFI_TARGET_HAS_INT128") | 2 months agoMar 8, 2026 |
| [make\_sunver.pl](https://github.com/libffi/libffi/blob/master/make_sunver.pl "make_sunver.pl") | [make\_sunver.pl](https://github.com/libffi/libffi/blob/master/make_sunver.pl "make_sunver.pl") | [Add missing build script, make\_sunver.pl.](https://github.com/libffi/libffi/commit/ca112537df7b9cdbccad7541aa3cb43b2a2dac9a "Add missing build script, make_sunver.pl.") | 7 years agoOct 26, 2019 |
| [msvcc.sh](https://github.com/libffi/libffi/blob/master/msvcc.sh "msvcc.sh") | [msvcc.sh](https://github.com/libffi/libffi/blob/master/msvcc.sh "msvcc.sh") | [fix windows tests (](https://github.com/libffi/libffi/commit/01b56f4b3e89a349228c4ccf55061d847153c8d6 "fix windows tests (#595)  * Update .appveyor.yml  * add (debug+release)*(shared+static) CI  * fix libversion") [#595](https://github.com/libffi/libffi/pull/595) [)](https://github.com/libffi/libffi/commit/01b56f4b3e89a349228c4ccf55061d847153c8d6 "fix windows tests (#595)  * Update .appveyor.yml  * add (debug+release)*(shared+static) CI  * fix libversion") | 5 years agoMar 24, 2021 |
| [stamp-h.in](https://github.com/libffi/libffi/blob/master/stamp-h.in "stamp-h.in") | [stamp-h.in](https://github.com/libffi/libffi/blob/master/stamp-h.in "stamp-h.in") | [Initial commit](https://github.com/libffi/libffi/commit/c6dddbd02bad9654ed58cdb0feb360934d105dec "Initial commit") | 17 years agoOct 4, 2009 |
| View all files |

## Repository files navigation

# Status

[Permalink: Status](https://github.com/libffi/libffi#status)

This is WIP repo for what will eventually become libffi-3.6.0.

# What is libffi?

[Permalink: What is libffi?](https://github.com/libffi/libffi#what-is-libffi)

Compilers for high level languages generate code that follow certain
conventions. These conventions are necessary, in part, for separate
compilation to work. One such convention is the "calling
convention". The "calling convention" is essentially a set of
assumptions made by the compiler about where function arguments will
be found on entry to a function. A "calling convention" also specifies
where the return value for a function is found.

Some programs may not know at the time of compilation what arguments
are to be passed to a function. For instance, an interpreter may be
told at run-time about the number and types of arguments used to call
a given function. Libffi can be used in such programs to provide a
bridge from the interpreter program to compiled code.

The libffi library provides a portable, high level programming
interface to various calling conventions. This allows a programmer to
call any function specified by a call interface description at run
time.

FFI stands for Foreign Function Interface. A foreign function
interface is the popular name for the interface that allows code
written in one language to call code written in another language. The
libffi library really only provides the lowest, machine dependent
layer of a fully featured foreign function interface. A layer must
exist above libffi that handles type conversions for values passed
between the two languages.

# Supported Platforms

[Permalink: Supported Platforms](https://github.com/libffi/libffi#supported-platforms)

Libffi has been ported to many different platforms.

At the time of release, the following basic configurations have been
tested:

| Architecture | Operating System | Compiler |
| --- | --- | --- |
| AArch64 (ARM64) | iOS | Clang |
| AArch64 | Linux | GCC |
| AArch64 | Windows | MSVC |
| Alpha | Linux | GCC |
| Alpha | Tru64 | GCC |
| ARC | Linux | GCC |
| ARC32 | Linux | GCC |
| ARC64 | Linux | GCC |
| ARM | Linux | GCC |
| ARM | iOS | GCC |
| ARM | Windows | MSVC |
| AVR32 | Linux | GCC |
| Blackfin | uClinux | GCC |
| CSKY | Linux | GCC |
| HPPA | HPUX | GCC |
| HPPA64 | HPUX | GCC |
| KVX | Linux | GCC |
| IA-64 | Linux | GCC |
| LoongArch32 | Linux | GCC |
| LoongArch64 | Linux | GCC |
| M68K | FreeMiNT | GCC |
| M68K | Linux | GCC |
| M68K | RTEMS | GCC |
| M88K | OpenBSD/mvme88k | GCC |
| Meta | Linux | GCC |
| MicroBlaze | Linux | GCC |
| MIPS | IRIX | GCC |
| MIPS | Linux | GCC |
| MIPS | RTEMS | GCC |
| MIPS64 | Linux | GCC |
| Moxie | Bare metal | GCC |
| OpenRISC | Linux | GCC |
| PowerPC 32-bit | AIX | GCC |
| PowerPC 32-bit | AIX | IBM XL C |
| PowerPC 64-bit | AIX | IBM XL C |
| PowerPC | AMIGA | GCC |
| PowerPC | Linux | GCC |
| PowerPC | Mac OSX | GCC |
| PowerPC | FreeBSD | GCC |
| PowerPC 64-bit | FreeBSD | GCC |
| PowerPC 64-bit | Linux ELFv1 | GCC |
| PowerPC 64-bit | Linux ELFv2 | GCC |
| RISC-V 32-bit | Linux | GCC |
| RISC-V 64-bit | Linux | GCC |
| S390 | Linux | GCC |
| S390X | Linux | GCC |
| SH3 | Linux | GCC |
| SH4 | Linux | GCC |
| SH5/SH64 | Linux | GCC |
| SPARC | Linux | GCC |
| SPARC | Solaris | GCC |
| SPARC | Solaris | Oracle Solaris Studio C |
| SPARC64 | Linux | GCC |
| SPARC64 | FreeBSD | GCC |
| SPARC64 | Solaris | Oracle Solaris Studio C |
| TILE-Gx/TILEPro | Linux | GCC |
| VAX | OpenBSD/vax | GCC |
| WASM32 | Emscripten | EMCC |
| WASM64 | Emscripten | EMCC |
| X86 | FreeBSD | GCC |
| X86 | GNU HURD | GCC |
| X86 | Interix | GCC |
| X86 | kFreeBSD | GCC |
| X86 | Linux | GCC |
| X86 | OpenBSD | GCC |
| X86 | OS/2 | GCC |
| X86 | Solaris | GCC |
| X86 | Solaris | Oracle Solaris Studio C |
| X86 | Windows/Cygwin | GCC |
| X86 | Windows/MinGW | GCC |
| X86-64 | DragonFly BSD | GCC |
| X86-64 | FreeBSD | GCC |
| X86-64 | Linux | GCC |
| X86-64 | Linux/x32 | GCC |
| X86-64 | OpenBSD | GCC |
| X86-64 | Solaris | Oracle Solaris Studio C |
| X86-64 | Windows/Cygwin | GCC |
| X86-64 | Windows/MinGW | GCC |
| X86-64 | Mac OSX | GCC |
| Xtensa | Linux | GCC |

Please send additional platform test results to
[libffi-discuss@sourceware.org](mailto:libffi-discuss@sourceware.org).

# Installing libffi

[Permalink: Installing libffi](https://github.com/libffi/libffi#installing-libffi)

First you must configure the distribution for your particular
system. Go to the directory you wish to build libffi in and run the
"configure" program found in the root directory of the libffi source
distribution. Note that building libffi requires a C99 compatible
compiler. If you're building libffi directly from git hosted sources,
configure won't exist yet; run ./autogen.sh first. This will require
that you install autoconf, automake, libtool and texinfo.

You may want to tell configure where to install the libffi library and
header files. To do that, use the `--prefix` configure switch. Libffi
will install under /usr/local by default.

If you want to enable extra run-time debugging checks use the the
`--enable-debug` configure switch. This is useful when your program dies
mysteriously while using libffi.

Another useful configure switch is `--enable-purify-safety`. Using this
will add some extra code which will suppress certain warnings when you
are using Purify with libffi. Only use this switch when using
Purify, as it will slow down the library.

If you don't want to build documentation, use the `--disable-docs`
configure switch.

It's also possible to build libffi on Windows platforms with
Microsoft's Visual C++ compiler. In this case, use the msvcc.sh
wrapper script during configuration like so:

```
path/to/configure CC=path/to/msvcc.sh CXX=path/to/msvcc.sh LD=link CPP="cl -nologo -EP" CXXCPP="cl -nologo -EP" CPPFLAGS="-DFFI_BUILDING_DLL"
```

For 64-bit Windows builds, use `CC="path/to/msvcc.sh -m64"` and
`CXX="path/to/msvcc.sh -m64"`. You may also need to specify
`--build` appropriately.

It is also possible to build libffi on Windows platforms with the LLVM
project's clang-cl compiler, like below:

```
path/to/configure CC="path/to/msvcc.sh -clang-cl" CXX="path/to/msvcc.sh -clang-cl" LD=link CPP="clang-cl -EP"
```

When building with MSVC under a MingW environment, you may need to
remove the line in configure that sets 'fix\_srcfile\_path' to a 'cygpath'
command. ('cygpath' is not present in MingW, and is not required when
using MingW-style paths.)

To build static library for ARM64 with MSVC using visual studio solution, msvc\_build folder have
aarch64/Ffi\_staticLib.sln
required header files in aarch64/aarch64\_include/

SPARC Solaris builds require the use of the GNU assembler and linker.
Point `AS` and `LD` environment variables at those tool prior to
configuration.

For iOS builds, the `libffi.xcodeproj` Xcode project is available.

Configure has many other options. Use `configure --help` to see them all.

Once configure has finished, type "make". Note that you must be using
GNU make. You can ftp GNU make from ftp.gnu.org:/pub/gnu/make .

To ensure that libffi is working as advertised, type "make check".
This will require that you have DejaGNU installed.

To install the library and header files, type `make install`.

# History

[Permalink: History](https://github.com/libffi/libffi#history)

See the git log for details at [http://github.com/libffi/libffi](http://github.com/libffi/libffi).

```
3.6.0 ???
    Add LoongArch32 support.
    Add RISC-V static trampoline support.
    Add aarch64 GCS (Guarded Control Stack) support.
    Add conditional target support for __int128.
    Fix closures using FFI_REGISTER ABI.
    Fix SH linker errors with __USER_LABEL_PREFIX__.
    Fix compilation for ARM Windows targets.
    Fix compilation for Cortex-A53.
    Fix test compilation for some Android platforms.
    Fix x86 ASAN compatibility for win64.
    Define WIN32_LEAN_AND_MEAN before including windows.h.
    Fix comments that trip up some toolchains.

3.5.2 Aug-2-2025
    Add wasm64 support.
    Add DragonFly BSD support.
    Ensure trampoline file descriptors are closed on exec.

3.5.1 Jun-10-2025
    Fix symbol versioning error.

3.5.0 Jun-8-2025
    Add FFI_VERSION_STRING and FFI_VERSION_NUMBER macros, as well
      as ffi_get_version() and ffi_get_version_number() functions.
    Add ffi_get_default_abi() and ffi_get_closure_size() functions.
    Fix closures on powerpc64-linux when statically linking.
    Mark the PA stack as non-executable.

3.4.8 Apr-9-2025
    Add static trampoline support for powerpc-linux (32-bit SYSV BE),
      powerpc64-linux (64-bit ELFv1 BE) and
      powerpc64le-linux (64-bit ELFv2 LE)
    Various x86-64 bug fixes (x32 ABI and improper memory access for
      small argument calls).
    Fix to enable pointer authentication for aarch64.

3.4.7 Feb-8-2025
    Add static trampoline support for Linux on s390x.
    Fix BTI support for ARM64.
    Support pointer authentication for ARM64.
    Fix ASAN compatibility.
    Fix x86-64 calls with 6 GP registers and some SSE registers.
    Miscellaneous fixes for ARC and Darwin ARM64.
    Fix OpenRISC or1k and Solaris 10 builds.
    Remove nios2 port.

3.4.6 Feb-18-2024
    Fix long double regression on mips64 and alpha.

3.4.5 Feb-15-2024
    Add support for wasm32.
    Add support for aarch64 branch target identification (bti).
    Add support for ARCv3: ARC32 & ARC64.
    Add support for HPPA64, and many HPPA fixes.
    Add support for Haikuos on PowerPC.
    Fixes for AIX, loongson, MIPS, power, sparc64, and x86 Darwin.

3.4.4 Oct-23-2022
    Important aarch64 fixes, including support for linux builds
      with Link Time Optimization (-flto).
    Fix x86 stdcall stack alignment.
    Fix x86 Windows msvc assembler compatibility.
    Fix moxie and or1k small structure args.

3.4.3 Sep-19-2022
    All struct args are passed by value, regardless of size, as per ABIs.
    Enable static trampolines for Cygwin.
    Add support for Loongson's LoongArch64 architecture.
    Fix x32 static trampolines.
    Fix 32-bit x86 stdcall stack corruption.
    Fix ILP32 aarch64 support.

3.4.2 Jun-28-2021
    Add static trampoline support for Linux on x86_64 and ARM64.
    Add support for Alibaba's CSKY architecture.
    Add support for Kalray's KVX architecture.
    Add support for Intel Control-flow Enforcement Technology (CET).
    Add support for ARM Pointer Authentication (PA).
    Fix 32-bit PPC regression.
    Fix MIPS soft-float problem.
    Enable tmpdir override with the $LIBFFI_TMPDIR environment variable.
    Enable compatibility with MSVC runtime stack checking.
    Reject float and small integer argument in ffi_prep_cif_var().
      Callers must promote these types themselves.

3.3 Nov-23-2019
    Add RISC-V support.
    New API in support of GO closures.
    Add IEEE754 binary128 long double support for 64-bit Power
    Default to Microsoft's 64 bit long double ABI with Visual C++.
    GNU compiler uses 80 bits (128 in memory) FFI_GNUW64 ABI.
    Add Windows on ARM64 (WOA) support.
    Add Windows 32-bit ARM support.
    Raw java (gcj) API deprecated.
    Add pre-built PDF documentation to source distribution.
    Many new test cases and bug fixes.

3.2.1 Nov-12-2014
    Build fix for non-iOS AArch64 targets.

3.2 Nov-11-2014
    Add C99 Complex Type support (currently only supported on
      s390).
    Add support for PASCAL and REGISTER calling conventions on x86
      Windows/Linux.
    Add OpenRISC and Cygwin-64 support.
    Bug fixes.

3.1 May-19-2014
    Add AArch64 (ARM64) iOS support.
    Add Nios II support.
    Add m88k and DEC VAX support.
    Add support for stdcall, thiscall, and fastcall on non-Windows
      32-bit x86 targets such as Linux.
    Various Android, MIPS N32, x86, FreeBSD and UltraSPARC IIi
      fixes.
    Make the testsuite more robust: eliminate several spurious
      failures, and respect the $CC and $CXX environment variables.
    Archive off the manually maintained ChangeLog in favor of git
      log.

3.0.13 Mar-17-2013
    Add Meta support.
    Add missing Moxie bits.
    Fix stack alignment bug on 32-bit x86.
    Build fix for m68000 targets.
    Build fix for soft-float Power targets.
    Fix the install dir location for some platforms when building
      with GCC (OS X, Solaris).
    Fix Cygwin regression.

3.0.12 Feb-11-2013
    Add Moxie support.
    Add AArch64 support.
    Add Blackfin support.
    Add TILE-Gx/TILEPro support.
    Add MicroBlaze support.
    Add Xtensa support.
    Add support for PaX enabled kernels with MPROTECT.
    Add support for native vendor compilers on
      Solaris and AIX.
    Work around LLVM/GCC interoperability issue on x86_64.

3.0.11 Apr-11-2012
    Lots of build fixes.
    Add support for variadic functions (ffi_prep_cif_var).
    Add Linux/x32 support.
    Add thiscall, fastcall and MSVC cdecl support on Windows.
    Add Amiga and newer MacOS support.
    Add m68k FreeMiNT support.
    Integration with iOS' xcode build tools.
    Fix Octeon and MC68881 support.
    Fix code pessimizations.

3.0.10 Aug-23-2011
    Add support for Apple's iOS.
    Add support for ARM VFP ABI.
    Add RTEMS support for MIPS and M68K.
    Fix instruction cache clearing problems on
      ARM and SPARC.
    Fix the N64 build on mips-sgi-irix6.5.
    Enable builds with Microsoft's compiler.
    Enable x86 builds with Oracle's Solaris compiler.
    Fix support for calling code compiled with Oracle's Sparc
      Solaris compiler.
    Testsuite fixes for Tru64 Unix.
    Additional platform support.

3.0.9 Dec-31-2009
    Add AVR32 and win64 ports.  Add ARM softfp support.
    Many fixes for AIX, Solaris, HP-UX, *BSD.
    Several PowerPC and x86-64 bug fixes.
    Build DLL for windows.

3.0.8 Dec-19-2008
    Add *BSD, BeOS, and PA-Linux support.

3.0.7 Nov-11-2008
    Fix for ppc FreeBSD.
    (thanks to Andreas Tobler)

3.0.6 Jul-17-2008
    Fix for closures on sh.
    Mark the sh/sh64 stack as non-executable.
    (both thanks to Kaz Kojima)

3.0.5 Apr-3-2008
    Fix libffi.pc file.
    Fix #define ARM for IcedTea users.
    Fix x86 closure bug.

3.0.4 Feb-24-2008
    Fix x86 OpenBSD configury.

3.0.3 Feb-22-2008
    Enable x86 OpenBSD thanks to Thomas Heller, and
      x86-64 FreeBSD thanks to Björn König and Andreas Tobler.
    Clean up test instruction in README.

3.0.2 Feb-21-2008
    Improved x86 FreeBSD support.
    Thanks to Björn König.

3.0.1 Feb-15-2008
    Fix instruction cache flushing bug on MIPS.
    Thanks to David Daney.

3.0.0 Feb-15-2008
    Many changes, mostly thanks to the GCC project.
    Cygnus Solutions is now Red Hat.

  [10 years go by...]

1.20 Oct-5-1998
    Raffaele Sena produces ARM port.

1.19 Oct-5-1998
    Fixed x86 long double and long long return support.
    m68k bug fixes from Andreas Schwab.
    Patch for DU assembler compatibility for the Alpha from Richard
      Henderson.

1.18 Apr-17-1998
    Bug fixes and MIPS configuration changes.

1.17 Feb-24-1998
    Bug fixes and m68k port from Andreas Schwab. PowerPC port from
    Geoffrey Keating. Various bug x86, Sparc and MIPS bug fixes.

1.16 Feb-11-1998
    Richard Henderson produces Alpha port.

1.15 Dec-4-1997
    Fixed an n32 ABI bug. New libtool, auto* support.

1.14 May-13-97
    libtool is now used to generate shared and static libraries.
    Fixed a minor portability problem reported by Russ McManus
    <mcmanr@eq.gs.com>.

1.13 Dec-2-1996
    Added --enable-purify-safety to keep Purify from complaining
      about certain low level code.
    Sparc fix for calling functions with < 6 args.
    Linux x86 a.out fix.

1.12 Nov-22-1996
    Added missing ffi_type_void, needed for supporting void return
      types. Fixed test case for non MIPS machines. Cygnus Support
      is now Cygnus Solutions.

1.11 Oct-30-1996
    Added notes about GNU make.

1.10 Oct-29-1996
    Added configuration fix for non GNU compilers.

1.09 Oct-29-1996
    Added --enable-debug configure switch. Clean-ups based on LCLint
    feedback. ffi_mips.h is always installed. Many configuration
    fixes. Fixed ffitest.c for sparc builds.

1.08 Oct-15-1996
    Fixed n32 problem. Many clean-ups.

1.07 Oct-14-1996
    Gordon Irlam rewrites v8.S again. Bug fixes.

1.06 Oct-14-1996
    Gordon Irlam improved the sparc port.

1.05 Oct-14-1996
    Interface changes based on feedback.

1.04 Oct-11-1996
    Sparc port complete (modulo struct passing bug).

1.03 Oct-10-1996
    Passing struct args, and returning struct values works for
    all architectures/calling conventions. Expanded tests.

1.02 Oct-9-1996
    Added SGI n32 support. Fixed bugs in both o32 and Linux support.
    Added "make test".

1.01 Oct-8-1996
    Fixed float passing bug in mips version. Restructured some
    of the code. Builds cleanly with SGI tools.

1.00 Oct-7-1996
    First release. No public announcement.
```

# Authors & Credits

[Permalink: Authors & Credits](https://github.com/libffi/libffi#authors--credits)

libffi was originally written by Anthony Green [green@moxielogic.com](mailto:green@moxielogic.com).

The developers of the GNU Compiler Collection project have made
innumerable valuable contributions. See the ChangeLog file for
details.

Some of the ideas behind libffi were inspired by Gianni Mariani's free
gencall library for Silicon Graphics machines.

The closure mechanism was designed and implemented by Kresten Krab
Thorup.

Major processor architecture ports were contributed by the following
developers:

```
aarch64             Marcus Shawcroft, James Greenhalgh
alpha               Richard Henderson
arc                 Hackers at Synopsis
arm                 Raffaele Sena
avr32               Bradley Smith
blackfin            Alexandre Keunecke I. de Mendonca
cris                Simon Posnjak, Hans-Peter Nilsson
csky                Ma Jun, Zhang Wenmeng
frv                 Anthony Green
ia64                Hans Boehm
kvx                 Yann Sionneau
loongarch           Cheng Lulu, Xi Ruoyao, Xu Hao,
                    Zhang Wenlong, Pan Xuefeng,
                    Meng Qinggang
m32r                Kazuhiro Inaoka
m68k                Andreas Schwab
m88k                Miod Vallat
metag               Hackers at Imagination Technologies
microblaze          Nathan Rossi
mips                Anthony Green, Casey Marshall
mips64              David Daney
moxie               Anthony Green
openrisc            Sebastian Macke
pa                  Randolph Chung, Dave Anglin, Andreas Tobler
pa64                Dave Anglin
powerpc             Geoffrey Keating, Andreas Tobler,
                    David Edelsohn, John Hornkvist
powerpc64           Jakub Jelinek
riscv               Michael Knyszek, Andrew Waterman, Stef O'Rear
s390                Gerhard Tonn, Ulrich Weigand
sh                  Kaz Kojima
sh64                Kaz Kojima
sparc               Anthony Green, Gordon Irlam
tile-gx/tilepro     Walter Lee
vax                 Miod Vallat
wasm32              Hood Chatham, Brion Vibber, Kleis Auke Wolthuizen
x86                 Anthony Green, Jon Beniston
x86-64              Bo Thorsen
xtensa              Chris Zankel
```

Jesper Skov and Andrew Haley both did more than their fair share of
stepping through the code and tracking down bugs.

Thanks also to Tom Tromey for bug fixes, documentation and
configuration help.

Thanks to Jim Blandy, who provided some useful feedback on the libffi
interface.

Andreas Tobler has done a tremendous amount of work on the testsuite.

Alex Oliva solved the executable page problem for SElinux.

The list above is almost certainly incomplete and inaccurate. I'm
happy to make corrections or additions upon request.

If you have a problem, or have found a bug, please file an issue on
our issue tracker at [https://github.com/libffi/libffi/issues](https://github.com/libffi/libffi/issues).

The author can be reached at [green@moxielogic.com](mailto:green@moxielogic.com).

To subscribe/unsubscribe to our mailing lists, visit:
[https://sourceware.org/mailman/listinfo/libffi-announce](https://sourceware.org/mailman/listinfo/libffi-announce) [https://sourceware.org/mailman/listinfo/libffi-discuss](https://sourceware.org/mailman/listinfo/libffi-discuss)

## About

A portable foreign-function interface library.


[sourceware.org/libffi](http://sourceware.org/libffi "http://sourceware.org/libffi")

### Resources

[Readme](https://github.com/libffi/libffi#readme-ov-file)

### License

Unknown, Unknown licenses found


### Licenses found

[Unknown\\
\\
LICENSE](https://github.com/libffi/libffi/blob/master/LICENSE) [Unknown\\
\\
LICENSE-BUILDTOOLS](https://github.com/libffi/libffi/blob/master/LICENSE-BUILDTOOLS)

### Uh oh!

There was an error while loading. [Please reload this page](https://github.com/libffi/libffi).

[Activity](https://github.com/libffi/libffi/activity)

[Custom properties](https://github.com/libffi/libffi/custom-properties)

### Stars

[**4.3k**\\
stars](https://github.com/libffi/libffi/stargazers)

### Watchers

[**89**\\
watching](https://github.com/libffi/libffi/watchers)

### Forks

[**814**\\
forks](https://github.com/libffi/libffi/forks)

[Report repository](https://github.com/contact/report-content?content_url=https%3A%2F%2Fgithub.com%2Flibffi%2Flibffi&report=libffi+%28user%29)

## [Releases\  17](https://github.com/libffi/libffi/releases)

[v3.5.2\\
Latest\\
\\
on Aug 2, 2025Aug 2, 2025](https://github.com/libffi/libffi/releases/tag/v3.5.2)

[\+ 16 releases](https://github.com/libffi/libffi/releases)

## [Packages\  0](https://github.com/orgs/libffi/packages?repo_name=libffi)

No packages published

### Uh oh!

There was an error while loading. [Please reload this page](https://github.com/libffi/libffi).

## [Contributors\  192](https://github.com/libffi/libffi/graphs/contributors)

- [![@atgreen](https://avatars.githubusercontent.com/u/89993?s=64&v=4)](https://github.com/atgreen)
- [![@rth7680](https://avatars.githubusercontent.com/u/2529319?s=64&v=4)](https://github.com/rth7680)
- [![@joshtriplett](https://avatars.githubusercontent.com/u/162737?s=64&v=4)](https://github.com/joshtriplett)
- [![@tromey](https://avatars.githubusercontent.com/u/1557670?s=64&v=4)](https://github.com/tromey)
- [![@zwaldowski](https://avatars.githubusercontent.com/u/170812?s=64&v=4)](https://github.com/zwaldowski)
- [![@landonf](https://avatars.githubusercontent.com/u/18884?s=64&v=4)](https://github.com/landonf)
- [![@oleavr](https://avatars.githubusercontent.com/u/735197?s=64&v=4)](https://github.com/oleavr)
- [![@bivab](https://avatars.githubusercontent.com/u/1488?s=64&v=4)](https://github.com/bivab)
- [![@hoodmane](https://avatars.githubusercontent.com/u/8739626?s=64&v=4)](https://github.com/hoodmane)
- [![@trofi](https://avatars.githubusercontent.com/u/226650?s=64&v=4)](https://github.com/trofi)
- [![@jeremyhu](https://avatars.githubusercontent.com/u/1258676?s=64&v=4)](https://github.com/jeremyhu)
- [![@freakboy3742](https://avatars.githubusercontent.com/u/37345?s=64&v=4)](https://github.com/freakboy3742)
- [![@angerman](https://avatars.githubusercontent.com/u/40449?s=64&v=4)](https://github.com/angerman)
- [![@mstorsjo](https://avatars.githubusercontent.com/u/69727?s=64&v=4)](https://github.com/mstorsjo)

[\+ 178 contributors](https://github.com/libffi/libffi/graphs/contributors)

## Languages

- [C70.1%](https://github.com/libffi/libffi/search?l=c)
- [Assembly20.8%](https://github.com/libffi/libffi/search?l=assembly)
- [M43.8%](https://github.com/libffi/libffi/search?l=m4)
- [Shell3.3%](https://github.com/libffi/libffi/search?l=shell)
- [Makefile0.7%](https://github.com/libffi/libffi/search?l=makefile)
- [Python0.6%](https://github.com/libffi/libffi/search?l=python)
- Other0.7%

You can’t perform that action at this time.