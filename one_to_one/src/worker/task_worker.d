module worker.task_worker;

@nogc nothrow:

// PORT-NOTE: CongeladoTaskFactory is a C-ABI factory function pointer (for dlopen).
alias CongeladoTaskFactory = extern(C) void* function();

class TaskInput {
  public:
    this(ref const(const(char)[][const(char)[]]) data) {
        m_data = &data;
    }

    bool has(const(char)[] key) const {
        // PORT-NOTE: AA lookup in @nogc — use linear scan or SwissHashMap in Run 2
        return (key in *m_data) !is null;
    }

    // PORT-NOTE: C++ was a template constrained to string/string_view/int/int64/double/bool.
    // D uses overloads. get_string / get_int / get_int64 / get_double / get_bool map to each.

    import util.optional : Optional, some, none;

    Optional!(const(char)[]) get_string(const(char)[] key) const {
        auto p = key in *m_data;
        if (p is null) return none!(const(char)[])();
        return some!(const(char)[])(*p);
    }

    Optional!int get_int(const(char)[] key) const {
        auto p = key in *m_data;
        if (p is null) return none!int();
        int val;
        if (parse_int(*p, val)) return some!int(val);
        return none!int();
    }

    Optional!long get_int64(const(char)[] key) const {
        auto p = key in *m_data;
        if (p is null) return none!long();
        long val;
        if (parse_long(*p, val)) return some!long(val);
        return none!long();
    }

    Optional!double get_double(const(char)[] key) const {
        auto p = key in *m_data;
        if (p is null) return none!double();
        double val;
        if (parse_double(*p, val)) return some!double(val);
        return none!double();
    }

    Optional!bool get_bool(const(char)[] key) const {
        auto p = key in *m_data;
        if (p is null) return none!bool();
        if (*p == "true")  return some!bool(true);
        if (*p == "false") return some!bool(false);
        return none!bool();
    }

    ref const(const(char)[][const(char)[]]) get_data_map() const { return *m_data; }

  private:
    // Lifetime: returned view is valid only while the source map passed to TaskInput lives.
    const(const(char)[][const(char)[]])* m_data;

    // Minimal @nogc parsers (no std.conv)
    static bool parse_int(const(char)[] s, out int val) {
        if (s.length == 0) return false;
        size_t i = 0;
        bool neg = false;
        if (s[0] == '-') { neg = true; ++i; }
        if (i == s.length) return false;
        long acc = 0;
        for (; i < s.length; ++i) {
            if (s[i] < '0' || s[i] > '9') return false;
            acc = acc * 10 + (s[i] - '0');
        }
        val = cast(int)(neg ? -acc : acc);
        return true;
    }

    static bool parse_long(const(char)[] s, out long val) {
        if (s.length == 0) return false;
        size_t i = 0;
        bool neg = false;
        if (s[0] == '-') { neg = true; ++i; }
        if (i == s.length) return false;
        long acc = 0;
        for (; i < s.length; ++i) {
            if (s[i] < '0' || s[i] > '9') return false;
            acc = acc * 10 + (s[i] - '0');
        }
        val = neg ? -acc : acc;
        return true;
    }

    static bool parse_double(const(char)[] s, out double val) {
        // PORT-NOTE: std::from_chars<double> — stubbed with simple C strtod-style fallback.
        // TODO: implement @nogc float parsing in Run 2.
        val = 0.0;
        return false;
    }
}

class TaskOutput {
  public:
    ref const(const(char)[][const(char)[]]) get_data() const { return m_data; }

    void set_string(const(char)[] key, const(char)[] val) {
        m_data[key] = val;
    }

    void set_bool(const(char)[] key, bool val) {
        m_data[key] = val ? "true" : "false";
    }

    // PORT-NOTE: C++ template `set<T>` — D uses overloads. Numeric formatting deferred.
    // TODO: add set_int / set_double variants with @nogc int→string conversion in Run 2.

  private:
    const(char)[][const(char)[]] m_data;
}

// PORT-NOTE: virtual interface with non-@nogc default implementations in C++.
// on_released / on_error returned std::function (GC in D); stubs return null.
interface ITaskWorker {
    const(char)[] get_task_type() const;
    TaskOutput execute(ref const(TaskInput) input);

    // Returns a release callback, or null if not implemented.
    void function() @nogc nothrow on_released() { return null; }

    // Returns an error callback, or null if not implemented.
    void function(void*) @nogc nothrow on_error() { return null; }
}
