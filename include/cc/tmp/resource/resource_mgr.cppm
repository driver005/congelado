module;

#include <memory>
#include <string>
#include <unordered_map>

export module cc_tmp:resource_resource_mgr;

import std;
import cc_abi;

export {

    namespace tensorflow {

        using ResourceHandle = ice::PjRtResource;

        class ResourceMgr
        {
        public:
            ResourceMgr() = default;
            ~ResourceMgr() = default;

            ResourceMgr(const ResourceMgr&) = delete;
            ResourceMgr& operator=(const ResourceMgr&) = delete;

            void InsertResource(const std::string& name, ice::PjRtResource resource)
            {
                m_resources[name] = std::move(resource);
            }

            ice::PjRtResource* FindResource(const std::string& name)
            {
                auto it = m_resources.find(name);
                if (it != m_resources.end()) {
                    return &it->second;
                }
                return nullptr;
            }

        private:
            std::unordered_map<std::string, ice::PjRtResource> m_resources;
        };

    } // namespace tensorflow

} // export
