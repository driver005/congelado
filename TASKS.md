# TASKS.md

## Objective
Refactor `Tensor`, `Shape`, `Buffer`, and `DataType` in the `intern` layer to follow the Factory pattern using `ice::sonic::RuntimeBase`. Apply C ABI renaming to strictly mirror the pattern used in `Logger`, completely eliminating the legacy C++ handle wrappers.

## Active Tasks
- [x] **Task 1: C API Renames** — 
    - Rename `TF_Tensor` → `TF_Tensor_Handle` and `TF_TensorOps` → `TF_Tensor`.
    - Rename `TF_Shape` → `TF_Shape_Handle` and `TF_ShapeOps` → `TF_Shape`.
    - Rename `TF_Buffer` → `TF_Buffer_Handle` and `TF_BufferOps` → `TF_Buffer`.
    - Rename `TF_DataType` (enum) → `TF_DataType_Enum` and `TF_DataTypeOps` → `TF_DataType`.
    - Perform a massive search-and-replace across the entire codebase (`include/c/`, `src/`, `include/yoshi/`) to update all function signatures to use `TF_Tensor_Handle*`, etc.
- [x] **Task 2: Builder Interfaces** — Rewrite `include/cc/abi/builder/intern/tensor.cppm` (and shape, buffer, datatype) to define abstract classes (e.g., `ice::builder::Tensor`). Implement `get_generic_vtable()` to return the new factory structs.
- [x] **Task 3: Sonic Runtimes** — Rewrite `include/cc/abi/sonic/intern/tensor.cppm` (and others) to define `ice::sonic::Tensor : public RuntimeBase<Tensor, ice::builder::Tensor, TF_Tensor, true>`. These act as the factories.
- [x] **Task 4: Codebase Update** — Remove `TensorRuntime`, `TensorBuilder`, etc., and replace them with direct factory invocations.
