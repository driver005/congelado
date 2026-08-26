module;

#include <memory>
#include <string>

export module cc_tmp:resource_resource_var;

import std;
import cc_abi;
import :tensor_tensor;

export {

    namespace tensorflow {

        class Var
        {
        public:
            Var() :
                m_tensor{},
                m_var_info{}
            {
            }

            explicit Var(Tensor tensor) :
                m_tensor{std::move(tensor)},
                m_var_info{}
            {
            }

            Tensor* tensor()
            {
                return &m_tensor;
            }

            const Tensor* tensor() const
            {
                return &m_tensor;
            }

            ice::VariableInfo& var_info()
            {
                return m_var_info;
            }

            const ice::VariableInfo& var_info() const
            {
                return m_var_info;
            }

        private:
            Tensor m_tensor;
            ice::VariableInfo m_var_info;
        };

    } // namespace tensorflow

} // export
