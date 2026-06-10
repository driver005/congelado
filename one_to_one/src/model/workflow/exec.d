module model.workflow.exec;

@nogc nothrow:

import model.common.identifiers : ExecutionId, CorrelationId;
import model.common.timestamps  : ExecutionTimings;
import model.workflow.status    : WorkflowStatus;
import model.task.instance      : TaskInstance, KvEntry;
import util.result              : Result;
import util.optional            : Optional;

// PORT-NOTE: C++ used std::unordered_map<std::string,std::string> for variables
// and std::vector<TaskInstance> for task_instances. D uses fixed-size arrays+count.
// Max 64 variables, max 128 task instances.

class WorkflowExecution {
  public:
    void add_task_instance(TaskInstance instance) nothrow {
        // PORT-NOTE: C++ used std::vector::push_back; D uses fixed buffer[128].
        assert(m_task_instances_count < 128);
        m_task_instances_buf[m_task_instances_count++] = instance;
    }
    void add_variable(const(char)[] key, const(char)[] value) nothrow {
        assert(m_variables_count < 64);
        m_variables_buf[m_variables_count++] = KvEntry(key, value);
    }

    void set_exec_id(ExecutionId execution_id) nothrow { m_exec_id     = execution_id; }
    void set_def_name(const(char)[] name)      nothrow { m_def_name    = name;         }
    void set_def_version(uint version_)        nothrow { m_def_version = version_;     }
    void set_status(WorkflowStatus status)     nothrow { m_status      = status;       }
    void set_correlation_id(Optional!CorrelationId correlation_id) nothrow {
        m_correlation_id = correlation_id;
    }
    void set_variables(KvEntry[] variables) nothrow {
        size_t n = variables.length < 64 ? variables.length : 64;
        m_variables_buf[0 .. n] = variables[0 .. n];
        m_variables_count = n;
    }
    void set_task_instances(TaskInstance[] instances) nothrow {
        size_t n = instances.length < 128 ? instances.length : 128;
        m_task_instances_buf[0 .. n] = instances[0 .. n];
        m_task_instances_count = n;
    }
    void set_timings(ExecutionTimings timings) nothrow { m_timings = timings; }

    const(char)[]                      get_def_name()        const nothrow { return m_def_name;    }
    const(ExecutionId)                 get_exec_id()         const nothrow { return m_exec_id;     }
    uint                               get_def_version()     const nothrow { return m_def_version; }
    WorkflowStatus                     get_status()          const nothrow { return m_status;      }
    const(Optional!CorrelationId)      get_correlation_id()  const nothrow { return m_correlation_id; }
    const(KvEntry)[]                   get_variables()       const nothrow {
        return cast(const(KvEntry)[]) m_variables_buf[0 .. m_variables_count];
    }
    const(TaskInstance)[]              get_task_instances()  const nothrow {
        return cast(const(TaskInstance)[]) m_task_instances_buf[0 .. m_task_instances_count];
    }
    const(ExecutionTimings)            get_timings()         const nothrow { return m_timings; }

    Result!(bool, const(char)[]) validate() const nothrow {
        if (m_def_name.length == 0)
            return Result!(bool, const(char)[]).err(
                "WorkflowExecution def_name must not be empty");
        if (m_def_version == 0)
            return Result!(bool, const(char)[]).err(
                "WorkflowExecution def_version must be at least 1");
        {
            auto result = m_timings.validate();
            if (!result.is_ok) return result;
        }
        foreach (ref instance; m_task_instances_buf[0 .. m_task_instances_count]) {
            auto result = instance.validate();
            if (!result.is_ok) return result;
        }
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    ExecutionId      m_exec_id;
    const(char)[]    m_def_name;
    uint             m_def_version = 1;
    WorkflowStatus   m_status      = WorkflowStatus.RUNNING;
    Optional!CorrelationId m_correlation_id;
    // PORT-NOTE: C++ std::unordered_map replaced by fixed KvEntry[64] buffer+count.
    KvEntry[64]       m_variables_buf;
    size_t            m_variables_count;
    // PORT-NOTE: C++ std::vector<TaskInstance> replaced by fixed buffer[128]+count.
    TaskInstance[128] m_task_instances_buf;
    size_t            m_task_instances_count;
    ExecutionTimings  m_timings;
}
