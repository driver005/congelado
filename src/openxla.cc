// OpenXLA hello world.
//
// Builds a vector-add kernel as stablehlo MLIR text with cc::stable_hlo::Builder
// (include/cc/stable_hlo), compiles it through PJRT's in-process CPU client
// (@xla//xla/pjrt/c:pjrt_c_api_cpu), feeds it two f32 vectors, executes it and
// prints the elementwise sum — a quick end-to-end check that the stable_hlo ->
// OpenXLA pipeline works.
//
// HOW TO RUN:
//   bazel run //src:openxla
//
// If your ~/.cache is read-only (e.g. in a sandbox), point bazel's state at a
// writable location instead:
//   bazel --output_user_root=/path/to/writable --repo_contents_cache= \
//       run --repository_cache=/path/to/writable //src:openxla
//
// Expected output:
//   kernel (stablehlo mlir):
//   module @add_kernel {
//     func.func @main(%arg0: tensor<4xf32>, %arg1: tensor<4xf32>) -> (tensor<4xf32>) {
//       %0 = stablehlo.add %arg0, %arg1 : tensor<4xf32>
//       return %0 : tensor<4xf32>
//     }
//   }
//   result: [11, 22, 33, 44]
//   PASS: 1+10=11, 2+20=22, 3+30=33, 4+40=44

import std;
import cc_stable_hlo;

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "xla/pjrt/c/pjrt_c_api.h"
#include "xla/pjrt/c/pjrt_c_api_cpu.h"

namespace {

// Prints a PJRT error and returns 1. `error` is destroyed.
int Fail(const PJRT_Api *api, const char *what, PJRT_Error *error) {
  PJRT_Error_Message_Args message_args{};
  message_args.struct_size = PJRT_Error_Message_Args_STRUCT_SIZE;
  message_args.error = error;
  api->PJRT_Error_Message(&message_args);
  std::fprintf(stderr, "%s failed: %.*s\n", what,
               static_cast<int>(message_args.message_size), message_args.message);
  PJRT_Error_Destroy_Args destroy_args{};
  destroy_args.struct_size = PJRT_Error_Destroy_Args_STRUCT_SIZE;
  destroy_args.error = error;
  api->PJRT_Error_Destroy(&destroy_args);
  return 1;
}

}  // namespace

int main() {
  using namespace cc::stable_hlo;

  // ---- 1. Build the add kernel as stablehlo MLIR text. --------------------
  Builder builder;
  Function function{"main"};
  function.add_parameter(Shape{{4}, DType::f32()});
  Parameter lhs = function.get_arguments().back(); // copy — a second add_parameter below could
                                                     // reallocate m_arguments and dangle a
                                                     // reference taken here
  function.add_parameter(Shape{{4}, DType::f32()});
  Parameter rhs = function.get_arguments().back(); // copy

  Operation op{"add", "binary"};
  op.append_parameter(lhs);
  op.append_parameter(rhs);
  op.append_result(Parameter{function.next_id(), lhs.get_shape()});
  function.add_op(op);
  auto results = function.get_op(function.get_op_count() - 1).get_results();
  function.set_returns({results.begin(), results.end()});

  Module module{"add_kernel"};
  module.append_function(function);
  builder.append_module(module);
  auto module_text = builder.build();
  if (!module_text) {
    std::fprintf(stderr, "build failed: %s\n", module_text.error().c_str());
    return 1;
  }
  std::printf("kernel (stablehlo mlir):\n%s\n", module_text->c_str());

  // ---- 2. PJRT CPU client. ------------------------------------------------
  const PJRT_Api *api = GetPjrtApi();

  PJRT_Client_Create_Args client_args{};
  client_args.struct_size = PJRT_Client_Create_Args_STRUCT_SIZE;
  if (PJRT_Error *error = api->PJRT_Client_Create(&client_args)) {
    return Fail(api, "PJRT_Client_Create", error);
  }
  PJRT_Client *client = client_args.client;

  // ---- 3. Topology + compile the MLIR module. -----------------------------
  PJRT_Client_TopologyDescription_Args topology_args{};
  topology_args.struct_size = PJRT_Client_TopologyDescription_Args_STRUCT_SIZE;
  topology_args.client = client;
  if (PJRT_Error *error = api->PJRT_Client_TopologyDescription(&topology_args)) {
    return Fail(api, "PJRT_Client_TopologyDescription", error);
  }
  PJRT_TopologyDescription *topology = topology_args.topology;

  PJRT_Program program{};
  program.struct_size = PJRT_Program_STRUCT_SIZE;
  program.code = const_cast<char *>(module_text->c_str());
  program.code_size = module_text->size();
  program.format = "mlir";
  program.format_size = 4;

  PJRT_Compile_Args compile_args{};
  compile_args.struct_size = PJRT_Compile_Args_STRUCT_SIZE;
  compile_args.topology = topology;
  compile_args.program = &program;
  compile_args.client = client;
  if (PJRT_Error *error = api->PJRT_Compile(&compile_args)) {
    return Fail(api, "PJRT_Compile", error);
  }
  PJRT_Executable *executable = compile_args.executable;

  // ---- 4. Load the executable for execution. ------------------------------
  PJRT_Client_Load_Args load_args{};
  load_args.struct_size = PJRT_Client_Load_Args_STRUCT_SIZE;
  load_args.client = client;
  load_args.executable = executable;
  if (PJRT_Error *error = api->PJRT_Client_Load(&load_args)) {
    return Fail(api, "PJRT_Client_Load", error);
  }
  PJRT_LoadedExecutable *loaded_executable = load_args.loaded_executable;

  // ---- 5. Device to execute on. -------------------------------------------
  PJRT_Client_Devices_Args devices_args{};
  devices_args.struct_size = PJRT_Client_Devices_Args_STRUCT_SIZE;
  devices_args.client = client;
  if (PJRT_Error *error = api->PJRT_Client_Devices(&devices_args)) {
    return Fail(api, "PJRT_Client_Devices", error);
  }
  PJRT_Device *device = devices_args.devices[0];

  // ---- 6. Transfer the two inputs to device buffers. ----------------------
  const float left[4] = {1.0f, 2.0f, 3.0f, 4.0f};
  const float right[4] = {10.0f, 20.0f, 30.0f, 40.0f};
  const int64_t dims[1] = {4};
  PJRT_Buffer *inputs[2] = {nullptr, nullptr};
  for (int i = 0; i < 2; ++i) {
    PJRT_Client_BufferFromHostBuffer_Args buffer_args{};
    buffer_args.struct_size = PJRT_Client_BufferFromHostBuffer_Args_STRUCT_SIZE;
    buffer_args.client = client;
    buffer_args.data = i == 0 ? left : right;
    buffer_args.type = PJRT_Buffer_Type_F32;
    buffer_args.dims = dims;
    buffer_args.num_dims = 1;
    buffer_args.host_buffer_semantics = PJRT_HostBufferSemantics_kImmutableOnlyDuringCall;
    buffer_args.device = device;
    if (PJRT_Error *error = api->PJRT_Client_BufferFromHostBuffer(&buffer_args)) {
      return Fail(api, "PJRT_Client_BufferFromHostBuffer", error);
    }
    inputs[i] = buffer_args.buffer;
  }

  // ---- 7. Execute. --------------------------------------------------------
  PJRT_Buffer *const arg_list[2] = {inputs[0], inputs[1]};
  PJRT_Buffer *const *argument_lists[1] = {arg_list};
  PJRT_Buffer *output_list[1] = {nullptr};
  PJRT_Buffer **output_lists[1] = {output_list};

  PJRT_LoadedExecutable_Execute_Args execute_args{};
  execute_args.struct_size = PJRT_LoadedExecutable_Execute_Args_STRUCT_SIZE;
  execute_args.executable = loaded_executable;
  execute_args.argument_lists = argument_lists;
  execute_args.num_devices = 1;
  execute_args.num_args = 2;
  execute_args.output_lists = output_lists;
  if (PJRT_Error *error = api->PJRT_LoadedExecutable_Execute(&execute_args)) {
    return Fail(api, "PJRT_LoadedExecutable_Execute", error);
  }
  PJRT_Buffer *result = output_list[0];

  // ---- 8. Read the result back to host. -----------------------------------
  PJRT_Buffer_ToHostBuffer_Args read_args{};
  read_args.struct_size = PJRT_Buffer_ToHostBuffer_Args_STRUCT_SIZE;
  read_args.src = result;
  if (PJRT_Error *error = api->PJRT_Buffer_ToHostBuffer(&read_args)) {
    return Fail(api, "PJRT_Buffer_ToHostBuffer(query)", error);
  }
  std::vector<float> output(read_args.dst_size / sizeof(float));
  read_args.dst = output.data();
  read_args.dst_size = output.size() * sizeof(float);
  if (PJRT_Error *error = api->PJRT_Buffer_ToHostBuffer(&read_args)) {
    return Fail(api, "PJRT_Buffer_ToHostBuffer", error);
  }

  std::printf("result: [%g, %g, %g, %g]\n", output[0], output[1], output[2], output[3]);

  const float expected[4] = {11.0f, 22.0f, 33.0f, 44.0f};
  bool ok = output.size() == 4;
  for (int i = 0; ok && i < 4; ++i) {
    ok = output[i] == expected[i];
  }
  std::printf(ok ? "PASS: 1+10=11, 2+20=22, 3+30=33, 4+40=44\n"
                 : "FAIL: result does not match [11, 22, 33, 44]\n");

  // ---- 9. Cleanup. --------------------------------------------------------
  PJRT_Buffer_Destroy_Args buffer_destroy{};
  buffer_destroy.struct_size = PJRT_Buffer_Destroy_Args_STRUCT_SIZE;
  buffer_destroy.buffer = result;
  api->PJRT_Buffer_Destroy(&buffer_destroy);
  for (PJRT_Buffer *input : inputs) {
    buffer_destroy.buffer = input;
    api->PJRT_Buffer_Destroy(&buffer_destroy);
  }
  PJRT_LoadedExecutable_Delete_Args delete_args{};
  delete_args.struct_size = PJRT_LoadedExecutable_Delete_Args_STRUCT_SIZE;
  delete_args.executable = loaded_executable;
  api->PJRT_LoadedExecutable_Delete(&delete_args);
  PJRT_Client_Destroy_Args destroy_args{};
  destroy_args.struct_size = PJRT_Client_Destroy_Args_STRUCT_SIZE;
  destroy_args.client = client;
  api->PJRT_Client_Destroy(&destroy_args);

  return ok ? 0 : 1;
}
