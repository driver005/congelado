# TASKS.md

## Objective
Refactor `Tensor`, `Shape`, `Buffer`, and `DataType` in the `intern` layer to mirror the
`Logger` extern-tier pattern: abstract `ice::builder::X` base + `get_generic_vtable()` +
`ice::sonic::Runtime<T, OpsStruct>` handle. Done — landed in commit 5ef5dbc.

## Active Tasks
- [x] **Task 1: C API Renames** —
    - `TF_Tensor` (handle) → `TF_Tensor_Handle`; vtable stays `TF_TensorOps`.
    - `TF_Shape` (handle) → `TF_Shape_Handle`; vtable `TF_ShapeOps` → `TF_Shape`.
    - `TF_Buffer` (handle) → `TF_Buffer_Handle`; vtable `TF_BufferOps` → `TF_Buffer`.
    - `TF_DataType` (enum) → `TF_DataType_Enum`; vtable stays `TF_DataTypeOps`.
    - Search-and-replace across `include/c/`, `src/`, `include/yoshi/` to use `TF_Tensor_Handle*`, etc.
    - Note: tensor/datatype vtables keep the `*Ops` suffix because `TF_Tensor` / `TF_DataType`
      are legacy TensorFlow C API public names (`api.h` uses `TF_Tensor*` as the tensor handle
      and `TF_DataType` as the enum). Renaming the vtable to those names caused a typedef
      conflict; see `docs/style-audit.md`.
- [x] **Task 2: Builder Interfaces** — `ice::builder::{Tensor, Shape, Buffer, DataType}`
    abstract classes with `get_generic_vtable()` returning the concrete vtable struct
    (`TF_TensorOps`, `TF_Shape`, `TF_Buffer`, `TF_DataTypeOps`).
- [x] **Task 3: Sonic Runtimes** — `ice::sonic::{Tensor, Shape, Buffer, DataType}` derive
    `ice::sonic::Runtime<T, OpsStruct>` (e.g. `Runtime<Tensor, TF_TensorOps>`). The earlier
    `RuntimeBase<T, Builder, CStruct, bool>` Factory variant never landed (no `RuntimeBase`
    in tree).
- [x] **Task 4: Codebase Update** — `TensorRuntime` / `TensorBuilder` etc. removed; calls go
    through the `Runtime`-derived factory + `resolve()`.
