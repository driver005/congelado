module;

#include <string>
#include <vector>

export module cc_tmp:ops_op;

import std;
import cc_abi;

export {

    namespace tensorflow {

        using OpDefBuilder = ice::OpDefinition;

        class OpRegistry
        {
        public:
            static OpRegistry* Global()
            {
                static OpRegistry s_registry;
                return &s_registry;
            }

            void Register(ice::OpDefinition&& op_def, ice::Status* status = nullptr)
            {
                TF_Status* c_status = nullptr;
                op_def.register_op(c_status);
            }
        };

    } // namespace tensorflow

} // export
