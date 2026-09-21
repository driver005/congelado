import os

store_dir = 'include/c/extern/store'
manager_h = os.path.join(store_dir, 'manager.h')
admin_h = os.path.join(store_dir, 'admin.h')
store_h = os.path.join(store_dir, 'store.h')

# 1. Rename manager.h -> admin.h and update its contents
with open(manager_h, 'r') as f:
    content = f.read()

content = content.replace('STORE_MANAGER', 'STORE_ADMIN')
content = content.replace('TFStoreManager', 'TFStoreAdmin')
content = content.replace('create_store_manager', 'create_store_admin')
content = content.replace('destroy_store_manager', 'destroy_store_admin')

with open(admin_h, 'w') as f:
    f.write(content)

os.remove(manager_h)

# 2. Update store.h
with open(store_h, 'r') as f:
    content = f.read()

content = content.replace('manager.h', 'admin.h')
content = content.replace('TFStoreManagerOps', 'TFStoreAdminOps')
content = content.replace('manager_ops', 'admin_ops')
content = content.replace('create_store_manager', 'create_store_admin')

with open(store_h, 'w') as f:
    f.write(content)

print("Renamed Manager to Admin successfully.")
