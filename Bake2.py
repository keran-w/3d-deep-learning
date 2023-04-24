import os
import sys
import subprocess

num_layer = sys.argv[1]
print(f'num_layer: {num_layer}')
os.makedirs(f'Dataset/ModelNet40-path-{num_layer}', exist_ok=True)

for path, subdirs, files in os.walk(os.curdir, topdown=True):
    subdirs.sort(reverse=True)
    for name in files:
        if not name.endswith('.off'):
            continue
        file_path = os.path.join(path, name)
        new_file_path = file_path.replace('ModelNet40', f'ModelNet40-path-{num_layer}').replace('.off', '.path')
        os.makedirs(new_file_path.rsplit('\\', 1)[0], exist_ok=True)
        if os.path.isfile(new_file_path):
            print(new_file_path, 'already exists')
        else:
            subprocess.run(['./GraphGenerator.exe', file_path, f'{num_layer}', '-p', new_file_path, '-r'])
