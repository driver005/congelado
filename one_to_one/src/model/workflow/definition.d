module model.workflow.definition;

@nogc nothrow:

import model.workflow.dag    : TaskNode, OutputMapping;
import model.common.policies : TimeoutPolicy;
import util.result           : Result;
import util.optional         : Optional;

// PORT-NOTE: C++ used std::vector for nodes, input_params, output_mappings.
// D uses fixed-size arrays+count. Max 64 nodes, 32 input params, 32 output mappings.

class WorkflowDef {
  public:
    void add_node(TaskNode node) nothrow {
        assert(m_nodes_count < 64);
        m_nodes_buf[m_nodes_count++] = node;
    }
    void add_input_param(const(char)[] param) nothrow {
        assert(m_input_params_count < 32);
        m_input_params_buf[m_input_params_count++] = param;
    }
    void add_output_mapping(OutputMapping mapping) nothrow {
        assert(m_output_mappings_count < 32);
        m_output_mappings_buf[m_output_mappings_count++] = mapping;
    }

    void set_name(const(char)[] name)               nothrow { m_name    = name;    }
    void set_version(uint version_)                 nothrow { m_version = version_; }
    void set_nodes(TaskNode[] nodes)                nothrow {
        size_t n = nodes.length < 64 ? nodes.length : 64;
        m_nodes_buf[0 .. n] = nodes[0 .. n];
        m_nodes_count = n;
    }
    void set_input_params(const(char)[][] params)   nothrow {
        size_t n = params.length < 32 ? params.length : 32;
        m_input_params_buf[0 .. n] = params[0 .. n];
        m_input_params_count = n;
    }
    void set_output_mappings(OutputMapping[] mappings) nothrow {
        size_t n = mappings.length < 32 ? mappings.length : 32;
        m_output_mappings_buf[0 .. n] = mappings[0 .. n];
        m_output_mappings_count = n;
    }
    void set_failure_workflow(Optional!(const(char)[]) failure_workflow) nothrow {
        m_failure_workflow = failure_workflow;
    }
    void set_timeout(Optional!TimeoutPolicy timeout_) nothrow { m_timeout = timeout_; }

    uint                            get_version()          const nothrow { return m_version; }
    const(char)[]                   get_name()             const nothrow { return m_name;    }
    const(TaskNode)[]               get_nodes()            const nothrow {
        return cast(const(TaskNode)[]) m_nodes_buf[0 .. m_nodes_count];
    }
    const(char)[][]                 get_input_params()     const nothrow {
        return cast(const(char)[][]) m_input_params_buf[0 .. m_input_params_count];
    }
    const(OutputMapping)[]          get_output_mappings()  const nothrow {
        return cast(const(OutputMapping)[]) m_output_mappings_buf[0 .. m_output_mappings_count];
    }
    const(Optional!(const(char)[])) get_failure_workflow() const nothrow { return m_failure_workflow; }
    const(Optional!TimeoutPolicy)   get_timeout()          const nothrow { return m_timeout;           }

    Result!(bool, const(char)[]) validate() const nothrow {
        if (m_name.length == 0)
            return Result!(bool, const(char)[]).err("WorkflowDef name must not be empty");
        if (m_version == 0)
            return Result!(bool, const(char)[]).err("WorkflowDef version must be at least 1");
        if (m_nodes_count == 0)
            return Result!(bool, const(char)[]).err("WorkflowDef must have at least one node");
        foreach (ref node; m_nodes_buf[0 .. m_nodes_count]) {
            auto r = node.validate();
            if (!r.is_ok) return r;
        }
        foreach (ref mapping; m_output_mappings_buf[0 .. m_output_mappings_count]) {
            auto r = mapping.validate();
            if (!r.is_ok) return r;
        }
        if (m_timeout.has_value) {
            auto r = m_timeout.value.validate();
            if (!r.is_ok) return r;
        }
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    const(char)[]    m_name;
    uint             m_version = 1;
    // PORT-NOTE: C++ std::vector replaced by fixed-size arrays+count.
    TaskNode[64]     m_nodes_buf;
    size_t           m_nodes_count;
    const(char)[][32] m_input_params_buf;
    size_t           m_input_params_count;
    OutputMapping[32] m_output_mappings_buf;
    size_t           m_output_mappings_count;
    Optional!(const(char)[]) m_failure_workflow;
    Optional!TimeoutPolicy   m_timeout;
}
