import os

# Files to modify:
# 1. include/c/extern/stream_executor/executor.h
#    Rename TF_StreamExecutor to TF_Executor and TF_StreamExecutorOps to TF_ExecutorOps
# 2. include/c/extern/stream_executor/platform.h
#    Rename TF_StreamExecutor to TF_Executor
# 3. include/c/extern/stream_executor/stream_executor.h
#    Rename TF_StreamExecutorFacade to TF_StreamExecutor
#    Update includes and ops references

def replace_in_file(filepath, replacements):
    with open(filepath, 'r') as f:
        content = f.read()
    for old, new in replacements:
        content = content.replace(old, new)
    with open(filepath, 'w') as f:
        f.write(content)

# 1. executor.h
replace_in_file('include/c/extern/stream_executor/executor.h', [
    ('TF_StreamExecutor', 'TF_Executor'),
    ('TF_STREAM_EXECUTOR', 'TF_EXECUTOR'),
    ('create_stream_executor', 'create_executor'),
    ('destroy_stream_executor', 'destroy_executor')
])

# 2. platform.h
replace_in_file('include/c/extern/stream_executor/platform.h', [
    ('TF_StreamExecutor', 'TF_Executor'),
    ('create_stream_executor_internal', 'create_executor_internal'),
    ('destroy_stream_executor_internal', 'destroy_executor_internal')
])

# 3. stream_executor.h
replace_in_file('include/c/extern/stream_executor/stream_executor.h', [
    ('TF_StreamExecutorFacade', 'TF_StreamExecutor'),
    ('TF_STREAM_EXECUTOR_FACADE', 'TF_STREAM_EXECUTOR'),
    ('create_stream_executor_facade', 'create_stream_executor'),
    ('destroy_stream_executor_facade', 'destroy_stream_executor'),
    ('init_stream_executor_facade', 'init_stream_executor'),
    ('TF_StreamExecutorOps', 'TF_ExecutorOps'),
    ('create_stream_executor(&executor_ops', 'create_executor(&executor_ops')
])
