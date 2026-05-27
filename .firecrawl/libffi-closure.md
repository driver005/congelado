Next: [Closure Example](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Closure-Example.html), Previous: [Multiple ABIs](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Multiple-ABIs.html), Up: [Using libffi](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Using-libffi.html)   \[ [Index](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Index.html "Index")\]

* * *

### 2.5 The Closure API

`libffi` also provides a way to write a generic function – a
function that can accept and decode any combination of arguments.
This can be useful when writing an interpreter, or to provide wrappers
for arbitrary functions.

This facility is called the _closure API_. Closures are not
supported on all platforms; you can check the `FFI_CLOSURES`
define to determine whether they are supported on the current
platform.

Because closures work by assembling a tiny function at runtime, they
require special allocation on platforms that have a non-executable
heap. Memory management for closures is handled by a pair of
functions:

Function: **void** _\*ffi\_closure\_alloc (size\_t size, void \*\*code)_

Allocate a chunk of memory holding size bytes. This returns a
pointer to the writable address, and sets \*code to the
corresponding executable address.

size should be sufficient to hold a `ffi_closure` object.

Function: **void** _ffi\_closure\_free (void \*writable)_

Free memory allocated using `ffi_closure_alloc`. The argument is
the writable address that was returned.

Once you have allocated the memory for a closure, you must construct a
`ffi_cif` describing the function call. Finally you can prepare
the closure function:

Function: **ffi\_status** _ffi\_prep\_closure\_loc (ffi\_closure \*closure, ffi\_cif \*cif, void (\*fun) (ffi\_cif \*cif, void \*ret, void \*\*args, void \*user\_data), void \*user\_data, void \*codeloc)_

Prepare a closure function. The arguments to
`ffi_prep_closure_loc` are:

closure

The address of a `ffi_closure` object; this is the writable
address returned by `ffi_closure_alloc`.

cif

The `ffi_cif` describing the function parameters. Note that this
object, and the types to which it refers, must be kept alive until the
closure itself is freed.

user\_data

An arbitrary datum that is passed, uninterpreted, to your closure
function.

codeloc

The executable address returned by `ffi_closure_alloc`.

fun

The function which will be called when the closure is invoked. It is
called with the arguments:

cif

The `ffi_cif` passed to `ffi_prep_closure_loc`.

ret

A pointer to the memory used for the function’s return value.

If the function is declared as returning `void`, then this value
is garbage and should not be used.

Otherwise, fun must fill the object to which this points,
following the same special promotion behavior as `ffi_call`.
That is, in most cases, ret points to an object of exactly the
size of the type specified when cif was constructed. However,
integral types narrower than the system register size are widened. In
these cases your program may assume that ret points to an
`ffi_arg` object.

args

A vector of pointers to memory holding the arguments to the function.

user\_data

The same user\_data that was passed to
`ffi_prep_closure_loc`.

`ffi_prep_closure_loc` will return `FFI_OK` if everything
went ok, and one of the other `ffi_status` values on error.

After calling `ffi_prep_closure_loc`, you can cast codeloc
to the appropriate pointer-to-function type.

You may see old code referring to `ffi_prep_closure`. This
function is deprecated, as it cannot handle the need for separate
writable and executable addresses.

* * *

Next: [Closure Example](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Closure-Example.html), Previous: [Multiple ABIs](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Multiple-ABIs.html), Up: [Using libffi](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Using-libffi.html)   \[ [Index](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Index.html "Index")\]