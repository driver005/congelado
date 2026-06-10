module model.task.instance;

@nogc nothrow:

import model.common.identifiers : TaskId, ExecutionId;
import model.common.timestamps  : ExecutionTimings;
import model.task.status        : TaskStatus;
import util.result              : Result;

// PORT-NOTE: C++ used std::unordered_map<std::string,std::string> for input/output data.
// D uses fixed-size KV pair arrays to avoid GC. Max 64 entries per map.

struct KvEntry {
    const(char)[] key;
    const(char)[] value;
}

class TaskInstance {
  public:
    void add_input_data(const(char)[] key, const(char)[] value) nothrow {
        // PORT-NOTE: C++ used unordered_map::emplace; D uses fixed buffer[64].
        assert(m_input_count < 64);
        m_input_buf[m_input_count++] = KvEntry(key, value);
    }
    void add_output_data(const(char)[] key, const(char)[] value) nothrow {
        assert(m_output_count < 64);
        m_output_buf[m_output_count++] = KvEntry(key, value);
    }

    void set_workflow_exec_id(ExecutionId execution_id) nothrow { m_workflow_exec_id = execution_id; }
    void set_def_name(const(char)[] def_name)           nothrow { m_def_name         = def_name;     }
    void set_task_id(TaskId task_id)                    nothrow { m_task_id          = task_id;      }
    void set_status(TaskStatus status)                  nothrow { m_status           = status;       }
    void set_seq(uint seq)                              nothrow { m_seq              = seq;          }
    void set_input_data(KvEntry[] data) nothrow {
        size_t n = data.length < 64 ? data.length : 64;
        m_input_buf[0 .. n] = data[0 .. n];
        m_input_count = n;
    }
    void set_output_data(KvEntry[] data) nothrow {
        size_t n = data.length < 64 ? data.length : 64;
        m_output_buf[0 .. n] = data[0 .. n];
        m_output_count = n;
    }
    void set_timings(ExecutionTimings timing) nothrow { m_timings     = timing; }
    void set_retry_count(uint count)          nothrow { m_retry_count = count;  }

    const(TaskId)          get_task_id()          const nothrow { return m_task_id;          }
    const(char)[]          get_def_name()          const nothrow { return m_def_name;          }
    const(ExecutionId)     get_workflow_exec_id()  const nothrow { return m_workflow_exec_id;  }
    TaskStatus             get_status()            const nothrow { return m_status;            }
    uint                   get_seq()               const nothrow { return m_seq;               }
    const(KvEntry)[]       get_input_data()        const nothrow {
        return cast(const(KvEntry)[]) m_input_buf[0 .. m_input_count];
    }
    const(KvEntry)[]       get_output_data()       const nothrow {
        return cast(const(KvEntry)[]) m_output_buf[0 .. m_output_count];
    }
    const(ExecutionTimings) get_timings()          const nothrow { return m_timings;           }
    uint                   get_retry_count()       const nothrow { return m_retry_count;       }

    Result!(bool, const(char)[]) validate() const nothrow {
        if (m_def_name.length == 0)
            return Result!(bool, const(char)[]).err("TaskInstance def_name must not be empty");
        auto r = m_timings.validate();
        if (!r.is_ok) return r;
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    TaskId           m_task_id;
    const(char)[]    m_def_name;
    ExecutionId      m_workflow_exec_id;
    TaskStatus       m_status     = TaskStatus.SCHEDULED;
    uint             m_seq        = 0;
    // PORT-NOTE: C++ std::unordered_map replaced by fixed KvEntry[64] buffer+count.
    KvEntry[64]      m_input_buf;
    size_t           m_input_count;
    KvEntry[64]      m_output_buf;
    size_t           m_output_count;
    ExecutionTimings m_timings;
    uint             m_retry_count = 0;
}
