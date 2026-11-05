"""Run in Blender: export the source FBX with its original UVs as MLOD P3D.

Uses the external Arma 3 Object Builder codec (GPL-3.0), unmodified:
https://github.com/MrClock8163/Arma3ObjectBuilder
"""
import argparse
import contextlib
import io
import json
import sys
import types
from pathlib import Path

import bpy

parser = argparse.ArgumentParser()
parser.add_argument('--fbx', required=True)
parser.add_argument('--physics', required=True)
parser.add_argument('--codec', required=True)
parser.add_argument('--output', required=True)
args = parser.parse_args(sys.argv[sys.argv.index('--') + 1:])
package = types.ModuleType('grz_p3d_codec')
package.__path__ = [str(Path(args.codec) / 'Arma3ObjectBuilder/io')]
sys.modules[package.__name__] = package
from grz_p3d_codec import data_p3d as p3d

bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
with contextlib.redirect_stdout(io.StringIO()):
    bpy.ops.import_scene.fbx(filepath=args.fbx)
bpy.context.view_layer.update()
meshes = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH']
assert meshes
visual = p3d.P3D_LOD()
visual.resolution.set(p3d.P3D_LOD_Resolution.VISUAL, 0)
source_uvs = []
material_counts = {}
for obj in meshes:
    mesh = obj.data
    assert mesh.uv_layers.active, obj.name
    mesh.calc_loop_triangles()
    vertex_offset = len(visual.verts)
    matrix = obj.matrix_world
    normal_matrix = matrix.to_3x3().inverted().transposed()
    # FBX imports at centimetre-scaled dimensions. Reconcile to the supplied
    # P3D's metre-scale container, while leaving all UV coordinates unchanged.
    for vertex in mesh.vertices:
        point = (matrix @ vertex.co) * 100
        visual.verts.append((point.x, point.y, point.z, 0))
    uv_data = mesh.uv_layers.active.data
    for triangle in mesh.loop_triangles:
        material_name = mesh.materials[triangle.material_index].name
        if 'containers_colors_03_co' in material_name:
            key = 'exterior'
        elif 'metal_white_lightrust' in material_name:
            key = 'metal'
        else:
            assert material_name.startswith('_NNN'), material_name
            key = 'interior'
        material_counts[key] = material_counts.get(key, 0) + 1
        indices, normals, uvs = [], [], []
        for vertex_id, loop_id in zip(triangle.vertices, triangle.loops):
            indices.append(vertex_offset + vertex_id)
            normal = (normal_matrix @ mesh.corner_normals[loop_id].vector).normalized()
            normals.append(len(visual.normals))
            visual.normals.append(tuple(normal))
            uv = uv_data[loop_id].uv
            source_uvs.append((float(uv.x), float(uv.y)))
            uvs.append((float(uv.x), 1 - float(uv.y)))
        texture = 'GRZ_Container\\data\\fbx_' + key + '_co.paa'
        material = 'GRZ_Container\\data\\fbx_' + key + '.rvmat'
        visual.faces.append([indices, normals, uvs, texture, material, 0])

source_physics = p3d.P3D_MLOD.read_file(args.physics)
physics = []
excluded = {p3d.P3D_LOD_Resolution.VISUAL, p3d.P3D_LOD_Resolution.SHADOW,
            p3d.P3D_LOD_Resolution.VIEW_GUNNER, p3d.P3D_LOD_Resolution.VIEW_PILOT,
            p3d.P3D_LOD_Resolution.VIEW_CARGO}
for lod in source_physics.lods:
    if lod.resolution.lod in excluded:
        continue
    for face in lod.faces:
        face[2] = [(u, 1 - v) for u, v in face[2]]
        if face[3].lower().startswith('grz_dm_containers'):
            face[3] = r'GRZ_Container\data\fbx_exterior_co.paa'
        if face[4].lower().startswith('grz_dm_containers'):
            face[4] = r'GRZ_Container\data\fbx_exterior.rvmat'
    physics.append(lod)
assert any(lod.resolution.lod == p3d.P3D_LOD_Resolution.GEOMETRY for lod in physics)
output = p3d.P3D_MLOD()
output.lods = [visual] + physics
output.write_file(args.output)
roundtrip = p3d.P3D_MLOD.read_file(args.output)
assert len(roundtrip.lods[0].faces) == len(visual.faces)
roundtrip_uvs = [uv for face in roundtrip.lods[0].faces for uv in face[2]]
assert len(roundtrip_uvs) == len(source_uvs)
maximum_error = max(abs(a - b) for uv_a, uv_b in zip(source_uvs, roundtrip_uvs) for a, b in zip(uv_a, uv_b))
assert maximum_error < 0.000001, maximum_error
bounds = [(min(point[a] for point in visual.verts), max(point[a] for point in visual.verts)) for a in range(3)]
assert 2 < bounds[0][1] - bounds[0][0] < 3
assert 5 < bounds[1][1] - bounds[1][0] < 7
assert 2 < bounds[2][1] - bounds[2][0] < 3
stats = {'source': '1A.fbx', 'visual_meshes': len(meshes), 'visual_vertices': len(visual.verts),
         'triangles': len(visual.faces), 'material_triangles': material_counts,
         'uv_roundtrip_max_error': maximum_error, 'bounds_m': bounds,
         'physics_lods_from_1mo': len(physics),
         'format': 'Editable MLOD P3D, not binarized ODOL. DayZ runtime validation required.'}
Path(args.output + '.json').write_text(json.dumps(stats, indent=2) + '\n')
print(json.dumps(stats))
