module model.workflow.event;

@nogc nothrow:

import model.common.identifiers : ExecutionId;
import util.result              : Result;
import util.optional            : Optional;

// PORT-NOTE: C++ used std::chrono::system_clock::time_point for m_issued_at.
// D port uses long (Unix epoch ms).

enum WorkflowEventType : ubyte {
    PAUSE,
    RESUME,
    TERMINATE,
    RESTART,
    SIGNAL,
}

class WorkflowEvent {
  public:
    void set_exec_id(ExecutionId execution_id)       nothrow { m_exec_id   = execution_id; }
    void set_type(WorkflowEventType type_)           nothrow { m_type      = type_;        }
    void set_payload(Optional!(const(char)[]) payload) nothrow { m_payload = payload;      }
    void set_issued_at(long tp)                      nothrow { m_issued_at = tp;           }

    WorkflowEventType                get_type()      const nothrow { return m_type;      }
    const(ExecutionId)               get_exec_id()   const nothrow { return m_exec_id;   }
    const(Optional!(const(char)[])) get_payload()   const nothrow { return m_payload;   }
    long                             get_issued_at() const nothrow { return m_issued_at; }

    Result!(bool, const(char)[]) validate() const nothrow {
        // Check nil UUID: all bytes zero
        bool is_nil = true;
        foreach (b; m_exec_id.data) {
            if (b != 0) { is_nil = false; break; }
        }
        if (is_nil)
            return Result!(bool, const(char)[]).err("WorkflowEvent exec_id must not be nil");
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    ExecutionId                   m_exec_id;
    WorkflowEventType             m_type;
    // PORT-NOTE: C++ used std::optional<std::string>; D uses Optional!(const(char)[]).
    Optional!(const(char)[])      m_payload;
    // PORT-NOTE: C++ used std::chrono::system_clock::time_point; D uses long (Unix epoch ms).
    long                          m_issued_at;
}
