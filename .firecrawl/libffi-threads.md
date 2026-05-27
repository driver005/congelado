Previous: [Closure Example](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Closure-Example.html), Up: [Using libffi](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Using-libffi.html)   \[ [Index](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Index.html "Index")\]

* * *

### 2.7 Thread Safety

`libffi` is not completely thread-safe. However, many parts are,
and if you follow some simple rules, you can use it safely in a
multi-threaded program.

- `ffi_prep_cif` may modify the `ffi_type` objects passed to
  it. It is best to ensure that only a single thread prepares a given
  `ffi_cif` at a time.

- On some platforms, `ffi_prep_cif` may modify the size and
  alignment of some types, depending on the chosen ABI. On these
  platforms, if you switch between ABIs, you must ensure that there is
  only one call to `ffi_prep_cif` at a time.


  Currently the only affected platform is PowerPC and the only affected
  type is `long double`.