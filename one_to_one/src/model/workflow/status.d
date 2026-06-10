module model.workflow.status;

@nogc nothrow:

enum WorkflowStatus : ubyte {
    RUNNING,
    COMPLETED,
    FAILED,
    TIMED_OUT,
    PAUSED,
    TERMINATED,
}

bool is_terminal(WorkflowStatus status) nothrow {
    return status == WorkflowStatus.COMPLETED || status == WorkflowStatus.FAILED
        || status == WorkflowStatus.TIMED_OUT || status == WorkflowStatus.TERMINATED;
}
