import os

gen_dir = 'include/c/extern/generator'
manager_h = os.path.join(gen_dir, 'manager.h')
catalog_h = os.path.join(gen_dir, 'catalog.h')
generator_h = os.path.join(gen_dir, 'generator.h')

# 1. Rename manager.h -> catalog.h and update its contents
with open(manager_h, 'r') as f:
    content = f.read()

content = content.replace('GENERATOR_MANAGER', 'GENERATOR_CATALOG')
content = content.replace('TFGeneratorManager', 'TFGeneratorCatalog')
content = content.replace('create_generator_manager', 'create_generator_catalog')
content = content.replace('destroy_generator_manager', 'destroy_generator_catalog')

with open(catalog_h, 'w') as f:
    f.write(content)

os.remove(manager_h)

# 2. Update generator.h
with open(generator_h, 'r') as f:
    content = f.read()

content = content.replace('manager.h', 'catalog.h')
content = content.replace('TFGeneratorManagerOps', 'TFGeneratorCatalogOps')
content = content.replace('manager_ops', 'catalog_ops')
content = content.replace('create_generator_manager', 'create_generator_catalog')

with open(generator_h, 'w') as f:
    f.write(content)

print("Renamed Manager to Catalog successfully.")
