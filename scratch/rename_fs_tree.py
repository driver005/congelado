import os

fs_dir = 'include/c/extern/filesystem'
basic_h = os.path.join(fs_dir, 'basic.h')
tree_h = os.path.join(fs_dir, 'tree.h')
filesystem_h = os.path.join(fs_dir, 'filesystem.h')

# 1. Rename basic.h -> tree.h and update its contents
with open(basic_h, 'r') as f:
    content = f.read()

content = content.replace('FILESYSTEM_BASIC', 'FILESYSTEM_TREE')
content = content.replace('TFFilesystemBasic', 'TFFilesystemTree')
content = content.replace('create_filesystem_basic', 'create_filesystem_tree')
content = content.replace('destroy_filesystem_basic', 'destroy_filesystem_tree')

with open(tree_h, 'w') as f:
    f.write(content)

os.remove(basic_h)

# 2. Update filesystem.h
with open(filesystem_h, 'r') as f:
    content = f.read()

content = content.replace('basic.h', 'tree.h')
content = content.replace('TFFilesystemBasicOps', 'TFFilesystemTreeOps')
content = content.replace('basic_ops', 'tree_ops')
content = content.replace('create_filesystem_basic', 'create_filesystem_tree')

with open(filesystem_h, 'w') as f:
    f.write(content)

print("Renamed Basic to Tree successfully.")
