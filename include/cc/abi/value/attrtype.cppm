module;

#include "c/intern/tf_attrtype.h"

export module cc_abi_value:attrtype;

export namespace ice {

enum class AttrType {
    String = TF_ATTR_STRING,
    Int = TF_ATTR_INT,
    Float = TF_ATTR_FLOAT,
    Bool = TF_ATTR_BOOL,
    Type = TF_ATTR_TYPE,
    Shape = TF_ATTR_SHAPE,
    Tensor = TF_ATTR_TENSOR,
    Placeholder = TF_ATTR_PLACEHOLDER,
    Func = TF_ATTR_FUNC,
};

} // namespace ice
