import os

fs_dir = 'include/c/extern/filesystem'
manager_h = os.path.join(fs_dir, 'manager.h')
basic_h = os.path.join(fs_dir, 'basic.h')
filesystem_h = os.path.join(fs_dir, 'filesystem.h')

# 1. Rename manager.h -> basic.h and update its contents
with open(manager_h, 'r') as f:
    content = f.read()

content = content.replace('FILESYSTEM_MANAGER', 'FILESYSTEM_BASIC')
content = content.replace('TF_FilesystemManager', 'TFFilesystemBasic')
content = content.replace('TF_FilesystemManagerOps', 'TFFilesystemBasicOps')
content = content.replace('create_filesystem_manager', 'create_filesystem_basic')
content = content.replace('destroy_filesystem_manager', 'destroy_filesystem_basic')

with open(basic_h, 'w') as f:
    f.write(content)

os.remove(manager_h)

# 2. Update filesystem.h
with open(filesystem_h, 'r') as f:
    content = f.read()

content = content.replace('manager.h', 'basic.h')
content = content.replace('TF_FilesystemManagerOps', 'TFFilesystemBasicOps')
content = content.replace('manager_ops', 'basic_ops')
content = content.replace('create_filesystem_manager', 'create_filesystem_basic')

with open(filesystem_h, 'w') as f:
    f.write(content)

print("Renamed and updated successfully.")
