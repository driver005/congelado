module;

#include <string>
#include <memory>
#include <functional>

export module cc_tmp:runtime_rendezvous;

import std;
import cc_abi;
import :tensor_tensor;

export {

namespace tensorflow {

class Rendezvous {
public:
    virtual ~Rendezvous() = default;

    virtual ice::Status Send(const std::string& key, const Tensor& val, bool is_dead) = 0;
    virtual void RecvAsync(const std::string& key, std::function<void(const ice::Status&, const Tensor&, bool)> done) = 0;
};

} // namespace tensorflow

} // export
