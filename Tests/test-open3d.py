import os
import cv2
import numpy as np
import pandas as pd
from pathlib import Path

import open3d as o3d

base_path = Path('./Dataset/')

metadata = pd.read_csv(base_path/'metadata_modelnet40.csv')

dataset_dir = base_path/'ModelNet40'


def read_off(file):
    if 'OFF' != file.readline().strip():
        raise ('Not a valid OFF header')
    n_verts, n_faces, __ = tuple(
        [int(s) for s in file.readline().strip().split(' ')])
    verts = [[float(s) for s in file.readline().strip().split(' ')]
             for i_vert in range(n_verts)]
    faces = [[int(s) for s in file.readline().strip().split(' ')][1:]
             for i_face in range(n_faces)]
    return verts, faces


# read in data
sample_label = 'airplane'
with open(f'{dataset_dir}/{sample_label}/train/{sample_label}_0001.off', 'r') as f:
    verts, faces = read_off(f)

m = o3d.geometry.TriangleMesh(o3d.open3d.utility.Vector3dVector(verts),
                              o3d.open3d.utility.Vector3iVector(faces))
m.compute_vertex_normals()


class texture:
    img = cv2.imread('./cupe_uv.png')
    width = img.shape[1]
    height = img.shape[0]
    
    def get_color_value(self, u, v):
		x = int(u * self.width)
		y = int(v * self.height)
		return self.img[y, x]


triangles_uv = []
for face in faces:
    # get the 3 vertices of the face
    v1, v2, v3 = verts[face[0]], verts[face[1]], verts[face[2]]

    # compute the texture coordinates for each vertex using the UV image
    uv1 = texture.get_color_value(
        v1[0] / texture.width, 1 - v1[1] / texture.height)
    uv2 = texture.get_color_value(
        v2[0] / texture.width, 1 - v2[1] / texture.height)
    uv3 = texture.get_color_value(
        v3[0] / texture.width, 1 - v3[1] / texture.height)

    # add the texture coordinates for each vertex to the triangle_uv list
    triangles_uv.append([uv1, uv2, uv3])


triangles_uv = np.array(triangles_uv)
print(triangles_uv.shape)  # (48, 2)

m.triangle_uvs = o3d.open3d.utility.Vector2dVector(triangles_uv)
m.triangle_material_ids = o3d.utility.IntVector([0]*len(faces))
m.textures = [o3d.geometry.Image(text)]

o3d.visualization.draw_geometries([m])
