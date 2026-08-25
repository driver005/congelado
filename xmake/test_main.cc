// Sole `main()` for every `*_test` target (see apply_test_target in xmake/common.lua). Keeping it
// in exactly one place matters mechanically: some core_packages (cpython) statically link a
// `main` of their own (Modules/main.c in libpython3.12.a); this object is compiled ahead of those
// static libs on the link line, so its `main` resolves the reference first and the archive's
// `main.o` is never pulled in.
//
// Explicitly runs boost::ut here rather than leaving it to its own default (registering suites at
// static-init time, then actually executing them lazily from `~runner()` — a global destructor
// that fires at `exit()`). That default execution point is a real hazard: it races against every
// other global's own atexit-registered teardown in unspecified order. Concretely, an early
// `Sha256::hash_hex()` unit test intermittently hit `EVP_DigestInit_ex` failing and then a
// segfault inside OpenSSL's error-string locking — `~runner()` was running after OpenSSL's own
// atexit cleanup had already torn down its internal state. Calling `.run()` here forces every
// suite to execute during normal `main()` flow instead, while every other global is still fully
// alive.
#ifdef CONGELADO_TEST
import boost.ut;

int main() {
    const bool failed = boost::ut::cfg<>.run({.report_errors = true});
    return failed ? 1 : 0;
}
#endif
