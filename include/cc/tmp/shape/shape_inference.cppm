module;

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

export module cc_tmp:shape_shape_inference;

import std;
import cc_abi;

export {

namespace tensorflow {
namespace shape_inference {

using ShapeHandle = ice::ShapeHandle;
using DimensionHandle = ice::DimensionHandle;
using InferenceContext = ice::ShapeInferenceContext;

} // namespace shape_inference
} // namespace tensorflow

} // export
