# libffi: a foreign function interface library

For Version 3.3 of libffi

## Anthony Green

* * *

This manual is for libffi, a portable foreign function interface library. Copyright c 2008–2019 Anthony Green and Red Hat, Inc. Permission is hereby granted, free of charge, to any person obtaining a copy of this soft- ware and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONIN- FRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

* * *

# 1 What is libffi?

Compilers for high level languages generate code that follow certain conventions. These conventions are necessary, in part, for separate compilation to work. One such convention is the calling convention. The calling convention is a set of assumptions made by the compiler about where function arguments will be found on entry to a function. A calling convention also specifies where the return value for a function is found. The calling convention is also sometimes called the ABI or Application Binary Interface. Some programs may not know at the time of compilation what arguments are to be passed to a function. For instance, an interpreter may be told at run-time about the number and types of arguments used to call a given function. ‘Libffi’ can be used in such programs to provide a bridge from the interpreter program to compiled code. The ‘libffi’ library provides a portable, high level programming interface to various calling conventions. This allows a programmer to call any function specified by a call interface description at run time. FFI stands for Foreign Function Interface. A foreign function interface is the popular name for the interface that allows code written in one language to call code written in another language. The ‘libffi’ library really only provides the lowest, machine dependent layer of a fully featured foreign function interface. A layer must exist above ‘libffi’ that handles type conversions for values passed between the two languages.

# 2 Using libffi

## 2.1 The Basics

‘Libffi’ assumes that you have a pointer to the function you wish to call and that you know the number and types of arguments to pass it, as well as the return type of the function. The first thing you must do is create an ffi\_cif object that matches the signature of the function you wish to call. This is a separate step because it is common to make multiple calls using a single ffi\_cif. The cif in ffi\_cif stands for Call InterFace. To prepare a call interface object, use the function ffi\_prep\_cif.

ffi\_status ffi prep cif (ffi cif \*cif, ffi abi abi, unsigned int nargs, \[Function\] ffi type \*rtype, ffi type \*\*argtypes) This initializes cif according to the given parameters. abi is the ABI to use; normally FFI\_DEFAULT\_ABI is what you want. Section 2.4 \[Multiple ABIs\], page 11, for more information. nargs is the number of arguments that this function accepts. rtype is a pointer to an ffi\_type structure that describes the return type of the function. See Section 2.3 \[Types\], page 3. argtypes is a vector of ffi\_type pointers. argtypes must have nargs elements. If nargs is 0, this argument is ignored. ffi\_prep\_cif returns a libffi status code, of type ffi\_status. This will be ei- ther FFI\_OK if everything worked properly; FFI\_BAD\_TYPEDEF if one of the ffi\_type objects is incorrect; or FFI\_BAD\_ABI if the abi parameter is invalid.

* * *

If the function being called is variadic (varargs) then ffi\_prep\_cif\_var must be used
instead of ffi\_prep\_cif.

ffi\_status ffi prep cif var (ffi cif \*cif, ffi abi abi, unsigned int \[Function\]
nfixedargs, unsigned int ntotalargs, ffi type \*rtype, ffi type
\*\*argtypes)
This initializes cif according to the given parameters for a call to a variadic function.

This initializes cif according to the given parameters for a call to a variadic function.
In general its operation is the same as for ffi\_prep\_cif except that:
nfixedargs is the number of fixed arguments, prior to any variadic arguments. It must

nfixedargs is the number of fixed arguments, prior to any variadic arguments. It must
be greater than zero.

ntotalargs the total number of arguments, including variadic and fixed arguments.
argtypes must have this many elements.
Note that, different cif’s must be prepped for calls to the same function when different

Note that, different cif’s must be prepped for calls to the same function when different
numbers of arguments are passed.

Also note that a call to ffi\_prep\_cif\_var with nfixedargs=nototalargs is NOT
equivalent to a call to ffi\_prep\_cif.

Note that the resulting ffi\_cif holds pointers to all the ffi\_type objects that were
used during initialization. You must ensure that these type objects have a lifetime at least
as long as that of the ffi\_cif.

To call a function using an initialized ffi\_cif, use the ffi\_call function:

void ffi call (ffi cif \*cif, void \*fn, void \*rvalue, void \*\*avalues) \[Function\]
This calls the function fn according to the description given in cif. cif must have
already been prepared using ffi\_prep\_cif.
rvalue is a pointer to a chunk of memory that will hold the result of the function call.

rvalue is a pointer to a chunk of memory that will hold the result of the function call.
This must be large enough to hold the result, no smaller than the system register size
(generally 32 or 64 bits), and must be suitably aligned; it is the caller’s responsibility
to ensure this. If cif declares that the function returns void (using ffi\_type\_void),
then rvalue is ignored.
In most situations, ‘libffi’ will handle promotion according to the ABI. However, for

then rvalue is ignored.
In most situations, ‘libffi’ will handle promotion according to the ABI. However, for
historical reasons, there is a special case with return values that must be handled by
your code. In particular, for integral (not struct) types that are narrower than the
system register size, the return value will be widened by ‘libffi’. ‘libffi’ provides
a type, ffi\_arg, that can be used as the return type. For example, if the CIF was
defined with a return type of char, ‘libffi’ will try to store a full ffi\_arg into the
return value.
avalues is a vector of void \* pointers that point to the memory locations holding the

Note that while the return value must be register-sized, arguments should exactly
match their declared type. For example, if an argument is a short, then the entry in
avalues should point to an object declared as short; but if the return type is short,
then rvalue should point to an object declared as a larger type – usually ffi\_arg.

* * *

### Chapter 2: Using libffi

# 2.2 Simple Example

Here is a trivial example that calls puts a few times.

#include <stdio.h> #include <ffi.h>

int main() { ffi\_cif cif; ffi\_type \*args\[1\]; void \*values\[1\]; char \*s; ffi\_arg rc;

/\\* Initialize the argument info vectors \*/ args\[0\] = &ffi\_type\_pointer; values\[0\] = &s;

/\\* Initialize the cif _/ if (ffi\_prep\_cif(&cif, FFI\_DEFAULT\_ABI, 1, &ffi\_type\_sint, args) == FFI\_OK) { s = "Hello World!"; ffi\_call(&cif, puts, &rc, values); /_ rc now holds the result of the call to puts \*/

/\\* values holds a pointer to the function’s arg, so to call puts() again all we need to do is change the value of s \*/ s = "This is cool!"; ffi\_call(&cif, puts, &rc, values); }

return 0; }

# 2.3 Types

## 2.3.1 Primitive Types

Libffi provides a number of built-in type descriptors that can be used to describe argument and return types:

ffi\_type\_void The type void. This cannot be used for argument types, only for return values.

ffi\_type\_uint8 An unsigned, 8-bit integer type.

* * *

## Chapter 2: Using libffi

ffi\_type\_sint8 A signed, 8-bit integer type.

ffi\_type\_uint16 An unsigned, 16-bit integer type.

ffi\_type\_sint16 A signed, 16-bit integer type.

ffi\_type\_uint32 An unsigned, 32-bit integer type.

ffi\_type\_sint32 A signed, 32-bit integer type.

ffi\_type\_uint64 An unsigned, 64-bit integer type.

ffi\_type\_sint64 A signed, 64-bit integer type.

ffi\_type\_float The C float type.

ffi\_type\_double The C double type.

ffi\_type\_uchar The C unsigned char type.

ffi\_type\_schar The C signed char type. (Note that there is not an exact equivalent to the C char type in libffi; ordinarily you should either use ffi\_type\_schar or ffi\_type\_uchar depending on whether char is signed.)

ffi\_type\_ushort The C unsigned short type.

ffi\_type\_sshort The C short type.

ffi\_type\_uint The C unsigned int type.

ffi\_type\_sint The C int type.

ffi\_type\_ulong The C unsigned long type.

ffi\_type\_slong The C long type.

ffi\_type\_longdouble On platforms that have a C long double type, this is defined. On other plat- forms, it is not.

* * *

ffi\_type\_pointer
A generic void \* pointer. You should use this for all pointers, regardless of
their real type.

ffi\_type\_complex\_float
The C \_Complex float type.

The C \_Complex float type.

ffi\_type\_complex\_double
The C \_Complex double type.

The C \_Complex double type.

ffi\_type\_complex\_longdouble
The C \_Complex long double type. On platforms that have a C long double

The C \_Complex long double type. On platforms that have a C long double
type, this is defined. On other platforms, it is not.

Each of these is of type ffi\_type, so you must take the address when passing to ffi\_
prep\_cif.

2.3.2 Structures

‘libffi’ is perfectly happy passing structures back and forth. You must first describe the
structure to ‘libffi’ by creating a new ffi\_type object for it.

ffi\_type
The ffi\_type has the following members:

\[Data type\]

The ffi\_type has the following members:

size\_t size
This is set by libffi; you should initialize it to zero.

This is set by libffi; you should initialize it to zero.

unsigned short alignment
This is set by libffi; you should initialize it to zero.

This is set by libffi; you should initialize it to zero.

unsigned short type
For a structure, this should be set to FFI\_TYPE\_STRUCT.

For a structure, this should be set to FFI\_TYPE\_STRUCT.

This is a ‘NULL’-terminated array of pointers to ffi\_type objects. There
is one element per field of the struct.
Note that ‘libffi’ has no special support for bit-fields. You must manage

ffi\_type \*\*elements
This is a ‘NULL’-terminated array of pointers to ffi\_type objects. There

The size or alignment of some of the built-in types may vary depending on the chosen
ABI.

2.3.3 Size and Alignment

Note that ‘libffi’ has no special support for bit-fields. You must manage
these manually.

The size and alignment of a new structure type will not be set by libffi until it has
been passed to ffi\_prep\_cif or ffi\_get\_struct\_offsets.

* * *

A structure type cannot be shared across ABIs. Instead each ABI needs its own copy
of the structure type.

So, before examining these fields, it is safest to pass the ffi\_type object to ffi\_prep\_
cif or ffi\_get\_struct\_offsets first. This function will do all the needed setup.

ffi\_type \*desired\_type;
ffi\_abi desired\_abi;
...
ffi\_cif cif;
if (ffi\_prep\_cif (&cif, desired\_abi, 0, desired\_type, NULL) == FFI\_OK)
{
size\_t size = desired\_type->size;
unsigned short alignment = desired\_type->alignment;
}
libffi also provides a way to get the offsets of the members of a structure.

ffi\_status ffi get struct offsets (ffi abi abi, ffi type \*struct type, \[Function\]
size t \*offsets)
Compute the offset of each element of the given structure type. abi is the ABI to use;
this is needed because in some cases the layout depends on the ABI.

offsets is an out parameter. The caller is responsible for providing enough space for
all the results to be written – one element per element type in struct type. If offsets
is NULL, then the type will be laid out but not otherwise modified. This can be useful
for accessing the type’s size or layout, as mentioned above.
This function returns FFI\_OK on success; FFI\_BAD\_ABI if abi is invalid; or FFI\_BAD\_

This function returns FFI\_OK on success; FFI\_BAD\_ABI if abi is invalid; or FFI\_BAD\_
TYPEDEF if struct type is invalid in some way. Note that only FFI\_STRUCT types are
valid here.

2.3.4 Arrays, Unions, and Enumerations

2.3.4.1 Arrays

‘libffi’ does not have direct support for arrays or unions. However, they can be emulated
using structures.
To emulate an array, simply create an ffi\_type using FFI\_TYPE\_STRUCT with as many

To emulate an array, simply create an ffi\_type using FFI\_TYPE\_STRUCT with as many
members as there are elements in the array.
ffi\_type array\_type;

ffi\_type array\_type;
ffi\_type \*\*elements
int i;
elements = malloc ((n + 1) \* sizeof (ffi\_type \*));
for (i = 0; i < n; ++i)
elements\[i\] = array\_element\_type;
elements\[n\] = NULL;
array\_type.size = array\_type.alignment = 0;
array\_type.type = FFI\_TYPE\_STRUCT;

* * *

array\_type.elements = elements;

Note that arrays cannot be passed or returned by value in C – structure types created
like this should only be used to refer to members of real FFI\_TYPE\_STRUCT objects.

However, a phony array type like this will not cause any errors from ‘libffi’ if you use
it as an argument or return type. This may be confusing.

2.3.4.2 Unions

One simple way to do this is to ensue that each element type is laid out. Then, give
the new structure type a single element; the size of the largest element; and the largest
alignment seen as well.

alignment seen as well.
This example uses the ffi\_prep\_cif trick to ensure that each element type is laid out.
ffi\_abi desired\_abi;
ffi\_type union\_type;
ffi\_type \*\*union\_elements;
int i;
ffi\_type element\_types\[2\];
element\_types\[1\] = NULL;
union\_type.size = union\_type.alignment = 0;
union\_type.type = FFI\_TYPE\_STRUCT;
union\_type.elements = element\_types;
for (i = 0; union\_elements\[i\]; ++i)
{
ffi\_cif cif;
if (ffi\_prep\_cif (&cif, desired\_abi, 0, union\_elements\[i\], NULL) == FFI\_OK)
{
if (union\_elements\[i\]->size > union\_type.size)
{
union\_type.size = union\_elements\[i\];
size = union\_elements\[i\]->size;
}
if (union\_elements\[i\]->alignment > union\_type.alignment)
union\_type.alignment = union\_elements\[i\]->alignment;
}
}

2.3.4.3 Enumerations flags such as -fshort-enums. See Section “Structures unions enumerations and bit-fields
implementation” in gcc, for more information about how GCC handles enumerations.

2.3.5 Type Example

The following example initializes a ffi\_type object representing the tm struct from Linux’s
time.h.

time.h.
Here is how the struct is defined:
struct tm {
int tm\_sec;
int tm\_min;
int tm\_hour;
int tm\_mday;
int tm\_mon;
int tm\_year;
int tm\_wday;
int tm\_yday;
int tm\_isdst;
/\\* Those are for future use. \*/
long int **tm\_gmtoff**;
\_\_const char \* **tm\_zone**;
};
Here is the corresponding code to describe this struct to libffi:
{
ffi\_type tm\_type;
ffi\_type _tm\_type\_elements\[12\];_
_int i;_
_tm\_type.size = tm\_type.alignment = 0;_
_tm\_type.type = FFI\_TYPE\_STRUCT;_
_tm\_type.elements = &tm\_type\_elements;_
_for (i = 0; i < 9; i++)_
_tm\_type\_elements\[i\] = &ffi\_type\_sint;_
_tm\_type\_elements\[9\] = &ffi\_type\_slong;_
_tm\_type\_elements\[10\] = &ffi\_type\_pointer;_
_tm\_type\_elements\[11\] = NULL;_
_/_ tm\_type can now be used to represent tm argument types
return types for ffi\_prep\_cif() \*/
}
2.3.6 Complex Types
‘libffi’ supports the complex types defined by the C99 standard (\_Complex float,

‘libffi’ supports the complex types defined by the C99 standard (\_Complex float, \_
Complex double and _Complex long double with the built-in type descriptors ffi\_type_
complex\_float, ffi\_type\_complex\_double and ffi\_type\_complex\_longdouble.

* * *

Custom complex types like \_Complex int can also be used. An ffi\_type object has to
be defined to describe the complex type to ‘libffi’.

ffi\_type \[Data type\]
size\_t size
This must be manually set to the size of the complex type.
unsigned short alignment
This must be manually set to the alignment of the complex type.
unsigned short type
For a complex type, this must be set to FFI\_TYPE\_COMPLEX.
ffi\_type \*\*elements
This is a ‘NULL’-terminated array of pointers to ffi\_type objects. The
first element is set to the ffi\_type of the complex’s base type. The
second element must be set to NULL.

The section Section 2.3.7 \[Complex Type Example\], page 9, shows a way to determine
the size and alignment members in a platform independent way.
For platforms that have no complex support in libffi yet, the functions ffi\_prep\_cif

For platforms that have no complex support in libffi yet, the functions ffi\_prep\_cif
and ffi\_prep\_args abort the program if they encounter a complex type.

2.3.7 Complex Type Example

This example demonstrates how to use complex types:

#include <stdio.h>
#include <ffi.h>
#include <complex.h>
void complex\_fn(\_Complex float cf,
\_Complex double cd,
\_Complex long double cld)
{
printf("cf=%f+%fi\\ncd=%f+%fi\\ncld=%f+%fi\\n",
(float)creal (cf), (float)cimag (cf),
(float)creal (cd), (float)cimag (cd),
(float)creal (cld), (float)cimag (cld));
}
int main()
{
ffi\_cif cif;
ffi\_type \*args\[3\];
void \*values\[3\];
\_Complex float cf;
\_Complex double cd;
\_Complex long double cld;

* * *

/\\* Initialize the argument info vectors _/_
_args\[0\] = &ffi\_type\_complex\_float;_
_args\[1\] = &ffi\_type\_complex\_double;_
_args\[2\] = &ffi\_type\_complex\_longdouble;_
_values\[0\] = &cf;_
_values\[1\] = &cd;_
_values\[2\] = &cld;_
_/_ Initialize the cif _/_
_if (ffi\_prep\_cif(&cif, FFI\_DEFAULT\_ABI, 3,_
_&ffi\_type\_void, args) == FFI\_OK)_
_{_
_cf = 1.0 + 20.0 \* I;_
_cd = 300.0 + 4000.0 \* I;_
_cld = 50000.0 + 600000.0 \* I;_
_/_ Call the function _/_
_ffi\_call(&cif, (void (_)(void))complex\_fn, 0, values);
}
return 0;
}

This is an example for defining a custom complex type descriptor for compilers that
support them:

/\*

- This macro can be used to define new complex type descriptors
- in a platform independent way.
- name: Name of the new descriptor is ffi\_type\_complex\_.
- type: The C base type of the complex type.
  \*/

#define FFI\_COMPLEX\_TYPEDEF(name, type, ffitype)

static ffi\_type \*ffi\_elements\_complex\_##name \[2\] = {

(ffi\_type \*)(&ffitype), NULL

};

struct struct\_align\_complex\_##name {

char c;

_Complex type x;_

_};_

_ffi\_type ffi\_type\_complex_##name = {

sizeof( _Complex type),_

_offsetof(struct struct\_align\_complex_##name, x),

FFI\_TYPE\_COMPLEX,

(ffi\_type \*\*)ffi\_elements\_complex\_##name

}
/\\* Define new complex type descriptors using the macro: \*/

/\\* Define new complex type descriptors using the macro: \*/

* * *

/\\* ffi\_type\_complex\_sint _/_
_FFI\_COMPLEX\_TYPEDEF(sint, int, ffi\_type\_sint);_
_/_ ffi\_type\_complex\_uchar \*/
FFI\_COMPLEX\_TYPEDEF(uchar, unsigned char, ffi\_type\_uint8);

The new type descriptors can then be used like one of the built-in type descriptors in
the previous example.

2.4 Multiple ABIs

A given platform may provide multiple different ABIs at once. For instance, the x86 platform has both ‘stdcall’ and ‘fastcall’ functions.

libffi provides some support for this. However, this is necessarily platform-specific.

2.5 The Closure API

libffi also provides a way to write a generic function – a function that can accept and
decode any combination of arguments. This can be useful when writing an interpreter, or
to provide wrappers for arbitrary functions.

This facility is called the closure API. Closures are not supported on all platforms; you
can check the FFI\_CLOSURES define to determine whether they are supported on the current
platform.

Because closures work by assembling a tiny function at runtime, they require special
allocation on platforms that have a non-executable heap. Memory management for closures
is handled by a pair of functions:

void \*ffi closure alloc (size t size, void \*\*code)
Allocate a chunk of memory holding size bytes. This returns a pointer to the writable

\[Function\]
Allocate a chunk of memory holding size bytes. This returns a pointer to the writable

Allocate a chunk of memory holding size bytes. This returns a pointer to the writable
address, and sets \*code to the corresponding executable address.

size should be sufficient to hold a ffi\_closure object.

void ffi closure free (void \*writable)
Free memory allocated using ffi\_closure\_alloc.

\[Function\]
The argument is the writable

Free memory allocated using ffi\_closure\_alloc. The argument is the writable
address that was returned.

ffi\_status ffi prep closure loc (ffi closure \*closure, ffi cif \*cif, \[Function\]
void (\*fun) (ffi cif \*cif, void \*ret, void \*\*args, void \*user\_data), void
\*user\_data, void \*codeloc)
Prepare a closure function. The arguments to ffi\_prep\_closure\_loc are:

Prepare a closure function. The arguments to ffi\_prep\_closure\_loc are:

closure The address of a ffi\_closure object; this is the writable address returned by ffi\_closure\_alloc.

cif The ffi\_cif describing the function parameters. Note that this object,
and the types to which it refers, must be kept alive until the closure itself
is freed.

* * *

## Chapter 2: Using libffi

user data An arbitrary datum that is passed, uninterpreted, to your closure func- tion.

codeloc The executable address returned by ffi\_closure\_alloc.

fun The function which will be called when the closure is invoked. It is called with the arguments:

## cif The ffi\_cif passed to ffi\_prep\_closure\_loc.

ret A pointer to the memory used for the function’s return value. If the function is declared as returning void, then this value is garbage and should not be used. Otherwise, fun must fill the object to which this points, fol- lowing the same special promotion behavior as ffi\_call. That is, in most cases, ret points to an object of exactly the size of the type specified when cif was constructed. How- ever, integral types narrower than the system register size are widened. In these cases your program may assume that ret points to an ffi\_arg object.

args A vector of pointers to memory holding the arguments to the function.

user data The same user data that was passed to ffi\_prep\_closure\_ loc.

ffi\_prep\_closure\_loc will return FFI\_OK if everything went ok, and one of the other ffi\_status values on error. After calling ffi\_prep\_closure\_loc, you can cast codeloc to the appropriate pointer-to-function type.

You may see old code referring to ffi\_prep\_closure. This function is deprecated, as it cannot handle the need for separate writable and executable addresses.

# 2.6 Closure Example

A trivial example that creates a new puts by binding fputs with stdout. #include <stdio.h> #include <ffi.h>

/\\* Acts like puts with the file given at time of enclosure. \*/ void puts\_binding(ffi\_cif \*cif, void _ret, void_ args\[\], void \*stream) { \*(ffi\_arg _)ret = fputs(_(char \*\*)args\[0\], (FILE \*)stream); }

typedef int (\*puts\_t)(char \*);

## int main()

* * *

## Chapter 2: Using libffi

{ ffi\_cif cif; ffi\_type \*args\[1\]; ffi\_closure \*closure;

void \*bound\_puts; int rc;

/\\* Allocate closure and bound\_puts \*/ closure = ffi\_closure\_alloc(sizeof(ffi\_closure), &bound\_puts);

if (closure) { /\* Initialize the argument info vectors \*/ args\[0\] = &ffi\_type\_pointer;

/\\* Initialize the cif _/ if (ffi\_prep\_cif(&cif, FFI\_DEFAULT\_ABI, 1, &ffi\_type\_sint, args) == FFI\_OK) { /_ Initialize the closure, setting stream to stdout _/ if (ffi\_prep\_closure\_loc(closure, &cif, puts\_binding, stdout, bound\_puts) == FFI\_OK) { rc = ((puts\_t)bound\_puts)("Hello World!"); /_ rc now holds the result of the call to fputs \*/ } } }

/\\* Deallocate both closure, and bound\_puts \*/ ffi\_closure\_free(closure);

return 0; }

# 2.7 Thread Safety

libffi is not completely thread-safe. However, many parts are, and if you follow some simple rules, you can use it safely in a multi-threaded program.

ffi\_prep\_cif may modify the ffi\_type objects passed to it. It is best to ensure that only a single thread prepares a given ffi\_cif at a time.

On some platforms, ffi\_prep\_cif may modify the size and alignment of some types, depending on the chosen ABI. On these platforms, if you switch between ABIs, you must ensure that there is only one call to ffi\_prep\_cif at a time.

* * *

Currently the only affected platform is PowerPC and the only affected type is long
double.

3 Missing Features

libffi is missing a few features. We welcome patches to add support for these.

Variadic closures.

There is no support for bit fields in structures.

The “raw” API is undocumented.

The Go API is undocumented.

Note that variadic support is very new and tested on a relatively small number of platforms.

Index

# A

ABI ..... 1
Application Binary Interface ..... 1

# C

calling convention ..... 1
cif ..... 1
closure API ..... 11
closures ..... 11

# F

ffi\_call ..... 2
ffi\_closure\_alloc ..... 11
ffi\_closure\_free ..... 11
ffi\_get\_struct\_offsets ..... 6
ffi\_prep\_cif ..... 1
ffi\_prep\_cif\_var ..... 2
ffi\_prep\_closure\_loc ..... 11
ffi\_status ..... 1, 2, 6, 11
ffi\_type ..... 5, 9
ffi\_type\_complex\_double ..... 5
ffi\_type\_complex\_float ..... 5
ffi\_type\_complex\_longdouble ..... 5
ffi\_type\_double ..... 4

ffi\_type\_float ..... 4
ffi\_type\_longdouble ..... 4
ffi\_type\_pointer ..... 5
ffi\_type\_schar ..... 4
ffi\_type\_sint ..... 4
ffi\_type\_sint16 ..... 4
ffi\_type\_sint32 ..... 4
ffi\_type\_sint64 ..... 4
ffi\_type\_sint8 ..... 4
ffi\_type\_slong ..... 4
ffi\_type\_sshort ..... 4
ffi\_type\_uchar ..... 4
ffi\_type\_uint ..... 4
ffi\_type\_uint16 ..... 4
ffi\_type\_uint32 ..... 4
ffi\_type\_uint64 ..... 4
ffi\_type\_uint8 ..... 3
ffi\_type\_ulong ..... 4
ffi\_type\_ushort ..... 4
ffi\_type\_void ..... 3
FFI ..... 1
FFI\_CLOSURES ..... 11
Foreign Function Interface ..... 1

V

void : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : 2, 11