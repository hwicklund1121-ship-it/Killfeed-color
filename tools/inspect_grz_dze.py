"""Read DayZ Editor v8 DZE geometry and strip private author metadata for packaging.
Field order follows public Dabs Framework EditorSaveData / EditorObjectData definitions.
Only file serialization is inspected; no uploaded code is executed.
"""
import argparse,struct,json,collections
from pathlib import Path
class Reader:
 def __init__(self,data):self.data=data;self.at=0
 def integer(self):
  n=struct.unpack_from('<i',self.data,self.at)[0];self.at+=4;return n
 def floating(self):
  n=struct.unpack_from('<f',self.data,self.at)[0];self.at+=4;return n
 def string(self):
  n=self.integer();assert 0<=n<=len(self.data)-self.at,(self.at,n)
  s=self.data[self.at:self.at+n].decode();self.at+=n;return s
 def vector(self):
  assert self.integer()==3
  return [self.floating() for _ in range(3)]
 def parameter(self,type):
  if type in ['string']:return self.string()
  if type in ['float']:return self.floating()
  if type in ['int','bool']:return self.integer()
  if type=='vector':return self.vector()
  if '<' in type:
   inner=type[type.index('<')+1:type.rindex('>')]
   if type.startswith('array<'):return [self.parameter(inner) for _ in range(self.integer())]
   if type.startswith('SerializableParam'):
    return [self.parameter(t.strip()) for t in inner.split(',')]
  raise ValueError('Unsupported serialized parameter type '+type)
 def object(self):
  obj=dict(Type=self.string(),DisplayName=self.string(),Position=self.vector(),Orientation=self.vector(),Scale=self.floating(),Flags=self.integer())
  obj['Parameters']={}
  for _ in range(self.integer()):
   key=self.string();kind=self.string();obj['Parameters'][key]=dict(Type=kind,Value=self.parameter(kind))
  for key in ['EditorOnly','Locked','AllowDamage','Simulate']:obj[key]=bool(self.integer())
  obj['Model']=self.string();obj['AttachmentMap']={}
  for _ in range(self.integer()):
   slot=self.integer();obj['AttachmentMap'][slot]=self.object()
  return obj

def read(path):
 data=Path(path).read_bytes();r=Reader(data)
 assert r.string()=='EditorBinned';assert r.integer()==8
 out=dict(MapName=r.string(),CameraPosition=r.vector())
 author_start=r.at;r.string()
 for _ in range(r.integer()):r.string()
 author_end=r.at
 r.integer();r.integer()
 out['EditorObjects']=[r.object() for _ in range(r.integer())]
 out['EditorHiddenObjects']=[]
 for _ in range(r.integer()):out['EditorHiddenObjects'].append(dict(Type=r.string(),Position=r.vector(),Flags=r.integer(),ModelName=r.string()))
 out['CameraTracks']=[]
 for _ in range(r.integer()):
  obj=r.object();obj['Time']=r.floating();out['CameraTracks'].append(obj)
 assert r.at==len(data),(r.at,len(data))
 sanitized=data[:author_start]+struct.pack('<ii',0,0)+data[author_end:]
 return out,sanitized
if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('dze',type=Path);parser.add_argument('--output',type=Path,default=Path('/tmp/grz-map-inspection.json'));args=parser.parse_args()
 out,sanitized=read(args.dze);args.output.write_text(json.dumps(out))
 print('Terrain:',out['MapName'],'objects:',len(out['EditorObjects']),'hidden:',len(out['EditorHiddenObjects']),'camera tracks:',len(out['CameraTracks']))
 print('Most common:',collections.Counter(o['Type'] for o in out['EditorObjects']).most_common(15))
 Path('/tmp/grz-map-sanitized.dze').write_bytes(sanitized)
