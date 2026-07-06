---
# PR: Worker macro usage: convert default workers to use CONGELADO_TASK

This branch updates the worker example tasks to use the new CONGELADO_TASK macro, which defines a per-translation-unit task instance and emits the worker lifecycle symbols that operate on that instance.

Changes included:
- Add CONGELADO_TASK usage to example/default worker source files under workers/ (one macro invocation per TU).
- Ensure the macro is placed at file scope after the task class definitions.

See: refactor/lifecycle-symbols
---
