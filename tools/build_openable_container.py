"""Prepare a UV-correct, animated container source project for DayZ Binarize.

The external Arma 3 Object Builder codec is used unmodified (GPL-3.0).
This prepares MLOD/model.cfg; it does not run the Windows DayZ binarizer.
"""
import argparse
import copy
import io
import json
import math
import re
import sys
import types
from collections import defaultdict
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile

from repair_hexgrid import read_pbo

ROOT = Path(__file__).resolve().parents[1]
NAME = 'GRZ_Container_Openable'
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--codec', required=True)
args = parser.parse_args()
package = types.ModuleType('grz_codec')
package.__path__ = [str(Path(args.codec) / 'Arma3ObjectBuilder/io')]
sys.modules['grz_codec'] = package
from grz_codec import data_p3d as p3d

_, original = read_pbo((ROOT / 'downloads/GRZ_Container.pbo').read_bytes())
visual = p3d.P3D_MLOD.read(io.BytesIO(original['model/container.p3d'])).lods[0]
parent = list(range(len(visual.verts)))
def find(index):
    while parent[index] != index:
        parent[index] = parent[parent[index]]
        index = parent[index]
    return index
for face in visual.faces:
    for vertex in face[0][1:]:
        parent[find(vertex)] = find(face[0][0])
groups = defaultdict(list)
for index in range(len(visual.verts)):
    groups[find(index)].append(index)
door_vertices = [set(), set()]
for indices in groups.values():
    points = [visual.verts[index] for index in indices]
    bounds = [(min(point[a] for point in points), max(point[a] for point in points)) for a in range(3)]
    if bounds[1][1] < -3.075 and bounds[0][0] >= -1.125 and bounds[0][1] <= 1.125 and bounds[2][0] >= 0.035 and bounds[2][1] <= 2.63 and not (bounds[0][0] < -0.08 and bounds[0][1] > 0.08):
        side = 0 if sum(point[0] for point in points) < 0 else 1
        door_vertices[side].update(indices)
assert all(100 < len(indices) < len(visual.verts) // 2 for indices in door_vertices)
assert not door_vertices[0].intersection(door_vertices[1])

def selection(lod, name, indices):
    indices = set(indices)
    tag = p3d.P3D_TAGG()
    tag.name = name
    tag.data = p3d.P3D_TAGG_DataSelection()
    tag.data.count_verts = len(lod.verts)
    tag.data.count_faces = len(lod.faces)
    tag.data.weight_verts = [(index, 1) for index in sorted(indices)]
    tag.data.weight_faces = [(index, 1) for index, face in enumerate(lod.faces) if set(face[0]).issubset(indices)]
    lod.taggs.append(tag)

for index, indices in enumerate(door_vertices, 1):
    selection(visual, 'door' + str(index), indices)
selection(visual, 'doorsTwin1', door_vertices[0] | door_vertices[1])
for face in visual.faces:
    face[2] = [(u, 1 - v) for u, v in face[2]]
    face[3] = face[3].replace('GRZ_Container\\', NAME + '\\')
    face[4] = face[4].replace('GRZ_Container\\', NAME + '\\')

geometry = p3d.P3D_LOD()
geometry.resolution.set(p3d.P3D_LOD_Resolution.GEOMETRY, 0)
geometry_doors = [set(), set()]
def box(bounds, number, door=None):
    x0,x1,y0,y1,z0,z1 = bounds
    offset = len(geometry.verts)
    points = [(x0,y0,z0),(x1,y0,z0),(x1,y1,z0),(x0,y1,z0),(x0,y0,z1),(x1,y0,z1),(x1,y1,z1),(x0,y1,z1)]
    geometry.verts.extend((*point,0) for point in points)
    for ids,normal in [([3,2,1,0],(0,0,-1)),([4,5,6,7],(0,0,1)),([0,1,5,4],(0,-1,0)),([2,3,7,6],(0,1,0)),([3,0,4,7],(-1,0,0)),([1,2,6,5],(1,0,0))]:
        normal_index = len(geometry.normals)
        geometry.normals.append(normal)
        geometry.faces.append([[offset+i for i in ids],[normal_index]*4,[(0,0)]*4,'',r'dz\data\data\penetration\ironplate.rvmat',0])
    ids = set(range(offset,offset+8))
    selection(geometry, 'component' + str(number).zfill(2), ids)
    if door is not None:
        geometry_doors[door].update(ids)
for i,bounds in enumerate([
    (-1.238,1.234,-3.17,3.084,-.034,.10),
    (-1.238,1.234,-3.17,3.084,2.56,2.639),
    (-1.238,-1.16,-3.17,3.084,.10,2.56),
    (1.156,1.234,-3.17,3.084,.10,2.56),
    (-1.16,1.156,3.004,3.084,.10,2.56),
    (-1.16,1.156,-3.17,-3.075,2.476,2.56)],1):
    box(bounds,i)
box((-1.116,-.004,-3.143,-3.096,.12,2.476),7,0)
box((.005,1.116,-3.143,-3.096,.12,2.476),8,1)
for i,indices in enumerate(geometry_doors,1):
    selection(geometry, 'door'+str(i),indices)
selection(geometry,'doorsTwin1',geometry_doors[0]|geometry_doors[1])
mass=p3d.P3D_TAGG();mass.name='#Mass#';mass.data=p3d.P3D_TAGG_DataMass();mass.data.masses=[25000/len(geometry.verts)]*len(geometry.verts);geometry.taggs.append(mass)
for key,value in [('class','house'),('autocenter','0')]:
    tag=p3d.P3D_TAGG();tag.name='#Property#';tag.data=p3d.P3D_TAGG_DataProperty();tag.data.key=key;tag.data.value=value;geometry.taggs.append(tag)
for tag in geometry.taggs:
    if isinstance(tag.data, p3d.P3D_TAGG_DataSelection):
        tag.data.count_verts=len(geometry.verts)
        tag.data.count_faces=len(geometry.faces)
view=copy.deepcopy(geometry);view.resolution.set(p3d.P3D_LOD_Resolution.VIEW_GEOMETRY,0)
fire=copy.deepcopy(geometry);fire.resolution.set(p3d.P3D_LOD_Resolution.FIRE_GEOMETRY,0)
road=p3d.P3D_LOD();road.resolution.set(p3d.P3D_LOD_Resolution.ROADWAY,0)
road.verts=[(-1.16,-3.17,.10,0),(1.156,-3.17,.10,0),(1.156,3.004,.10,0),(-1.16,3.004,.10,0)]
road.normals=[(0,0,1)];road.faces=[[[0,1,2,3],[0]*4,[(0,0)]*4,r'dz\surfaces\data\roadway\metal_thin_ext.paa','',0]]
memory=p3d.P3D_LOD();memory.resolution.set(p3d.P3D_LOD_Resolution.MEMORY,0)
memory.verts=[(-1.116,-3.12,.12,0),(-1.116,-3.12,2.476,0),(1.116,-3.12,.12,0),(1.116,-3.12,2.476,0),(0,-3.30,1.25,0)]
selection(memory,'door1_axis',[0,1]);selection(memory,'door2_axis',[2,3]);selection(memory,'doorsTwin1_action',[4])
model=p3d.P3D_MLOD();model.lods=[visual,geometry,memory,road,view,fire]
source=ROOT/'packaging/container-openable'/NAME
(source/'model').mkdir(parents=True,exist_ok=True)
(source/'$PBOPREFIX$').write_text(NAME+'\n')
model_path=source/'model/container_openable.p3d';model.write_file(str(model_path))
check=p3d.P3D_MLOD.read_file(str(model_path))
assert len(check.lods)==6
assert all(any(tag.name=='door1' for tag in lod.taggs) and any(tag.name=='door2' for tag in lod.taggs) for lod in [check.lods[0],check.lods[1],check.lods[4],check.lods[5]])
for name,data in original.items():
    if name.startswith('data/'):
        if name.endswith('.rvmat'):
            data=data.replace(b'GRZ_Container\\',NAME.encode()+b'\\')
        target=source/name;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
stats={'class':NAME,'visual_door_vertices':[len(ids) for ids in door_vertices],
       'door_collision_components':2,'static_collision_components':6,'lods':6,
       'native_action':'Twin doors open/close together','requires_dayz_binarize':True,
       'runtime_tested':False}
(source.parent/'MODEL-CHECKS.json').write_text(json.dumps(stats,indent=2)+'\n')
target=ROOT/'downloads/GRZ_Container_Openable-Builder.zip'
with ZipFile(target,'w',ZIP_DEFLATED) as archive:
    for path in source.parent.rglob('*'):
        if path.is_file() and not any(part.startswith('BuildOutput') for part in path.relative_to(source.parent).parts):
            archive.write(path,str(path.relative_to(source.parent)))
with ZipFile(target) as archive:assert archive.testzip() is None
print(json.dumps(stats,indent=2))
print('Source/model.cfg project packed. Compile with the included Windows builder before installing.')
