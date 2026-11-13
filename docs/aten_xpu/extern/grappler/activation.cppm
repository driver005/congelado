module;

#include "cc/proto/attr_value.pb.h"
#include "cc/proto/node_def.pb.h"

export module aten_xpu_extern_grappler:activation;

import std;

export namespace aten_xpu {

// How a fused kernel runs a TensorFlow activation node.
class SyclFusedActivation
{
public:
    // The activation as the kernels name it: a unary string (with an algorithm and scalars where
    // it needs them) and, for the ops that take an integer code, that code (0 when none fits).
    class Description
    {
    public:
        std::string unary;
        std::string algorithm;
        std::vector<float> scalars;
        std::int64_t code{0};
    };

    std::optional<Description> describe(const tensorflow::NodeDef& node) const
    {
        const std::string& op = node.op();
        if (op == "Relu") {
            return make("relu", "", {}, 1);
        }
        if (op == "Sigmoid") {
            return make("sigmoid", "", {}, 3);
        }
        if (op == "Tanh") {
            return make("tanh", "", {}, 4);
        }
        if (op == "Swish" || op == "Silu") {
            return make("swish", "", {}, 5);
        }
        if (op == "Relu6") {
            return make("hardtanh", "", {0.0F, 6.0F}, 0);
        }
        if (op == "LeakyRelu") {
            return make("leaky_relu", "", {floatAttr(node, "alpha", 0.2F)}, 0);
        }
        if (op == "Gelu") {
            const bool approximate = boolAttr(node, "approximate", false);
            return make("gelu", approximate ? "tanh" : "", {}, approximate ? 0 : 2);
        }
        return std::nullopt;
    }

private:
    static Description make(
        std::string_view unary,
        std::string_view algorithm,
        std::initializer_list<float> scalars,
        std::int64_t code
    )
    {
        Description description;
        description.unary = unary;
        description.algorithm = algorithm;
        description.scalars.assign(scalars.begin(), scalars.end());
        description.code = code;
        return description;
    }

    static float floatAttr(const tensorflow::NodeDef& node, const std::string& name, float fallback)
    {
        const auto found = node.attr().find(name);
        return found == node.attr().end() ? fallback : found->second.f();
    }

    static bool boolAttr(const tensorflow::NodeDef& node, const std::string& name, bool fallback)
    {
        const auto found = node.attr().find(name);
        return found == node.attr().end() ? fallback : found->second.b();
    }
};

} // namespace aten_xpu
