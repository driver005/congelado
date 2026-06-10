module model.task.status;

@nogc nothrow:

enum TaskStatus : ubyte {
    SCHEDULED,
    IN_PROGRESS,
    COMPLETED,
    FAILED,
    TIMED_OUT,
    SKIPPED,
    CANCELED,
}

enum TaskType : ubyte {
    SIMPLE,
    FORK,
    JOIN,
    SWITCH,
    SUB_WORKFLOW,
}

enum TaskResult : ubyte {
    SUCCESS,
    FAILURE,
    TIMEOUT,
    SKIPPED,
}

bool is_terminal(TaskStatus status) nothrow {
    return status == TaskStatus.COMPLETED || status == TaskStatus.FAILED
        || status == TaskStatus.TIMED_OUT || status == TaskStatus.SKIPPED
        || status == TaskStatus.CANCELED;
}
