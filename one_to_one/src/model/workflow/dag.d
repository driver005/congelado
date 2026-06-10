module model.workflow.dag;

@nogc nothrow:

import util.result : Result;

// PORT-NOTE: C++ used std::vector for edges and mappings; D uses fixed-size arrays+count.
// Max 16 mappings per edge, max 16 edges per node.

// ─── InputMapping ─────────────────────────────────────────────────────────────

class InputMapping {
  public:
    this() nothrow {}
    this(const(char)[] source, const(char)[] target) nothrow {
        m_source = source;
        m_target = target;
    }

    void set_source(const(char)[] source) nothrow { m_source = source; }
    void set_target(const(char)[] target) nothrow { m_target = target; }

    const(char)[] get_source() const nothrow { return m_source; }
    const(char)[] get_target() const nothrow { return m_target; }

    Result!(bool, const(char)[]) validate() const nothrow {
        if (m_source.length == 0)
            return Result!(bool, const(char)[]).err("InputMapping source must not be empty");
        if (m_target.length == 0)
            return Result!(bool, const(char)[]).err("InputMapping target must not be empty");
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    const(char)[] m_source;
    const(char)[] m_target;
}

// ─── OutputMapping ────────────────────────────────────────────────────────────

class OutputMapping {
  public:
    this() nothrow {}
    this(const(char)[] source, const(char)[] target) nothrow {
        m_source = source;
        m_target = target;
    }

    void set_source(const(char)[] source) nothrow { m_source = source; }
    void set_target(const(char)[] target) nothrow { m_target = target; }

    const(char)[] get_source() const nothrow { return m_source; }
    const(char)[] get_target() const nothrow { return m_target; }

    Result!(bool, const(char)[]) validate() const nothrow {
        if (m_source.length == 0)
            return Result!(bool, const(char)[]).err("OutputMapping source must not be empty");
        if (m_target.length == 0)
            return Result!(bool, const(char)[]).err("OutputMapping target must not be empty");
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    const(char)[] m_source;
    const(char)[] m_target;
}

// ─── TaskEdge ─────────────────────────────────────────────────────────────────

import util.optional : Optional;

class TaskEdge {
  public:
    void add_mapping(InputMapping mapping) nothrow {
        // PORT-NOTE: C++ used std::vector::push_back; D uses fixed buffer[16].
        assert(m_mappings_count < 16);
        m_mappings_buf[m_mappings_count++] = mapping;
    }
    void set_to(const(char)[] to_)               nothrow { m_to        = to_;      }
    void set_from(const(char)[] from_)           nothrow { m_from      = from_;    }
    void set_condition(Optional!(const(char)[]) cond) nothrow { m_condition = cond; }
    void set_mappings(InputMapping[] mappings)   nothrow {
        size_t n = mappings.length < 16 ? mappings.length : 16;
        m_mappings_buf[0 .. n] = mappings[0 .. n];
        m_mappings_count = n;
    }

    const(char)[]                 get_from()      const nothrow { return m_from;      }
    const(char)[]                 get_to()        const nothrow { return m_to;        }
    const(InputMapping)[]         get_mappings()  const nothrow {
        return cast(const(InputMapping)[]) m_mappings_buf[0 .. m_mappings_count];
    }
    const(Optional!(const(char)[]))  get_condition() const nothrow { return m_condition; }

    Result!(bool, const(char)[]) validate() const nothrow {
        if (m_from.length == 0)
            return Result!(bool, const(char)[]).err("TaskEdge from must not be empty");
        if (m_to.length == 0)
            return Result!(bool, const(char)[]).err("TaskEdge to must not be empty");
        foreach (ref m; m_mappings_buf[0 .. m_mappings_count]) {
            auto r = m.validate();
            if (!r.is_ok) return r;
        }
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    const(char)[]              m_from;
    const(char)[]              m_to;
    Optional!(const(char)[])   m_condition;
    // PORT-NOTE: C++ std::vector replaced by fixed InputMapping[16] buffer+count.
    InputMapping[16]           m_mappings_buf;
    size_t                     m_mappings_count;
}

// ─── TaskNode ─────────────────────────────────────────────────────────────────

class TaskNode {
  public:
    void add_edge(TaskEdge edge) nothrow {
        // PORT-NOTE: C++ used std::vector::push_back; D uses fixed buffer[16].
        assert(m_edges_count < 16);
        m_edges_buf[m_edges_count++] = edge;
    }
    void set_task_def_name(const(char)[] def_name) nothrow { m_def_name = def_name; }
    void set_edges(TaskEdge[] edges) nothrow {
        size_t n = edges.length < 16 ? edges.length : 16;
        m_edges_buf[0 .. n] = edges[0 .. n];
        m_edges_count = n;
    }

    const(char)[]       get_def_name() const nothrow { return m_def_name; }
    const(TaskEdge)[]   get_edges()    const nothrow {
        return cast(const(TaskEdge)[]) m_edges_buf[0 .. m_edges_count];
    }

    Result!(bool, const(char)[]) validate() const nothrow {
        if (m_def_name.length == 0)
            return Result!(bool, const(char)[]).err("TaskNode def_name must not be empty");
        foreach (ref e; m_edges_buf[0 .. m_edges_count]) {
            auto r = e.validate();
            if (!r.is_ok) return r;
        }
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    const(char)[] m_def_name;
    // PORT-NOTE: C++ std::vector replaced by fixed TaskEdge[16] buffer+count.
    TaskEdge[16]  m_edges_buf;
    size_t        m_edges_count;
}
