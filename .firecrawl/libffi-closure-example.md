Next: [Thread Safety](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Thread-Safety.html), Previous: [The Closure API](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/The-Closure-API.html), Up: [Using libffi](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Using-libffi.html)   \[ [Index](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Index.html "Index")\]

* * *

### 2.6 Closure Example

A trivial example that creates a new `puts` by binding
`fputs` with `stdout`.

```
#include <stdio.h>
#include <ffi.h>

/* Acts like puts with the file given at time of enclosure. */
void puts_binding(ffi_cif *cif, void *ret, void* args[],
                  void *stream)
{
  *(ffi_arg *)ret = fputs(*(char **)args[0], (FILE *)stream);
}

typedef int (*puts_t)(char *);

int main()
{
  ffi_cif cif;
  ffi_type *args[1];
  ffi_closure *closure;

  void *bound_puts;
  int rc;

  /* Allocate closure and bound_puts */
  closure = ffi_closure_alloc(sizeof(ffi_closure), &bound_puts);

  if (closure)
    {
      /* Initialize the argument info vectors */
      args[0] = &ffi_type_pointer;

      /* Initialize the cif */
      if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1,
                       &ffi_type_sint, args) == FFI_OK)
        {
          /* Initialize the closure, setting stream to stdout */
          if (ffi_prep_closure_loc(closure, &cif, puts_binding,
                                   stdout, bound_puts) == FFI_OK)
            {
              rc = ((puts_t)bound_puts)("Hello World!");
              /* rc now holds the result of the call to fputs */
            }
        }
    }

  /* Deallocate both closure, and bound_puts */
  ffi_closure_free(closure);

  return 0;
}
```