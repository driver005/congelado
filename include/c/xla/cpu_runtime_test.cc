// End-to-end XLA CPU integration test.  This creates an HLO vector-add kernel,
// compiles it through the local XLA client, executes it, and verifies the data
// transferred back from the runtime.

#include <cstdint>
#include <iostream>
#include <memory>

#include "xla/client/client_library.h"
#include "xla/hlo/builder/xla_builder.h"
#include "xla/literal_util.h"
#include "xla/shape_util.h"

namespace {

int Fail(const absl::Status& status) {
  std::cerr << status << '\n';
  return 1;
}

}  // namespace

int main() {
  xla::XlaBuilder builder("vector_add_runtime_test");
  const xla::Shape shape = xla::ShapeUtil::MakeShape(xla::S32, {4});
  const xla::XlaOp lhs = xla::Parameter(&builder, 0, shape, "lhs");
  const xla::XlaOp rhs = xla::Parameter(&builder, 1, shape, "rhs");
  xla::Add(lhs, rhs);

  auto computation = builder.Build();
  if (!computation.ok()) return Fail(computation.status());

  auto client = xla::ClientLibrary::GetOrCreateLocalClient();
  if (!client.ok()) return Fail(client.status());

  const xla::Literal left = xla::LiteralUtil::CreateR1<int32_t>({1, 2, 3, 4});
  const xla::Literal right = xla::LiteralUtil::CreateR1<int32_t>({10, 20, 30, 40});
  auto left_device = (*client)->TransferToServer(left);
  if (!left_device.ok()) return Fail(left_device.status());
  auto right_device = (*client)->TransferToServer(right);
  if (!right_device.ok()) return Fail(right_device.status());

  auto result = (*client)->ExecuteAndTransfer(
      *computation, {left_device->get(), right_device->get()});
  if (!result.ok()) return Fail(result.status());

  constexpr int32_t kExpected[] = {11, 22, 33, 44};
  for (int64_t i = 0; i < 4; ++i) {
    if (result->Get<int32_t>({i}) != kExpected[i]) {
      std::cerr << "unexpected result at index " << i << '\n';
      return 1;
    }
  }
  return 0;
}
