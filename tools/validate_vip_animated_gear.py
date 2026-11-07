"""Check VIP-only animated gear routing, grants, asset integrity and black/white frames."""
from pathlib import Path
from zipfile import ZipFile
import hashlib,json,io,struct
import numpy as np
from PIL import Image
from repair_hexgrid import read_pbo,section
r=Path(__file__).resolve().parents[1]
server=(r/'GRZ_Loadout/scripts/4_World/GRZLoadoutServer.c').read_text()
branch=server.split('if (rpc_type == GRZLoadoutRPC.VIP_GEAR_APPLY)',1)[1].split('if (rpc_type == GRZLoadoutRPC.SAVE_CLOTHING)',1)[0]
assert branch.index('!GRZVipLoadoutPolicy.Allowed(sender)')<branch.index('GRZLoadoutClothingServer.Equip')
assert 'sender.GetId() != GetIdentity().GetId()' in server
assert server.index('if (m_GRZDuelMatch != "")')<server.index('if (rpc_type == GRZLoadoutRPC.VIP_GEAR_APPLY)')
assert 'gearChoices[selectedSlot] = gearType' in branch and 'gearChoices.Insert("")' in branch
assert 'GRZOutfitStore.Snapshot' in branch and 'priorOutfit.Character' in branch
assert 'if (!GRZVipLoadoutPolicy.PaidWrap(availableWrap)) personal.Wraps.Insert' in server
assert 'if (personal.VipAuthorized) personal.VipWraps.Insert' in server
assert 'if (personal.VipAuthorized) personal.VipClothing.Insert' in server
assert 'page.Insert(freeClothes[clothingIndex])' in server
policy=(r/'GRZ_Loadout/scripts/4_World/GRZVipLoadoutPolicy.c').read_text()
assert 'override bool CanReceiveAttachment(EntityAI attachment, int slotId)' in policy and 'attachment.IsClothing()' in policy
clothes=(r/'GRZ_Loadout/scripts/4_World/GRZLoadoutClothingServer.c').read_text()
equip=clothes.split('static bool Equip(PlayerBase player, array<string> choices)',1)[1]
assert equip.index('foreach (string requested : choices)')<equip.index('array<EntityAI> fresh')
menu=(r/'GRZ_Killfeed/scripts/5_Mission/GRZ_Killfeed/GRZKillfeedMenu.c').read_text()
assert 'm_GRZGunChoices.Insert(-gearIndex - 1)' in menu and 'if (m_GRZGunCategory == 3)' in menu
assert 'gunCatalogueIndex < 0 || gunCatalogueIndex >= catalogue.VipGuns.Count()' in menu
assert 'VIP_GEAR_APPLY' in menu and 'catalogue.VipWraps' in menu and 'AnimatedTab' not in menu
_,base=read_pbo((r/'downloads/HEXGRID.pbo').read_bytes());baseconfig=base['config.cpp'].decode();a,_,_,b=section(baseconfig,'GlobalArmor')
import re
basearmour=re.sub(r'\s+','',baseconfig[a:b])
with ZipFile(r/'downloads/GRZ-Arena-Menu-Update.zip') as z:
 assert len([n for n in z.namelist() if n.endswith('.pbo')])==13
 for addon in ['GRZ_Loadout','GRZ_Killfeed','GRZ_AnimatedHexgrid','GRZ_AnimatedHexgridWhite','GRZ_AnimatedHexgridBlackV2']:
  data=z.read(addon+'.pbo');ext,files=read_pbo(data)
  assert ext['prefix']==addon and hashlib.sha1(data[:-21]).digest()==data[-20:]
  expected={str(p.relative_to(r/addon)):p.read_bytes() for p in (r/addon).rglob('*') if p.is_file() and p.name!='$PBOPREFIX$'}
  assert files==expected
  if addon.startswith('GRZ_AnimatedHexgrid'):
   cfg=files['config.cpp'].decode();assert 'GRZVipOnly = 1;' in cfg
   a,_,_,b=section(cfg,'GlobalArmor');assert re.sub(r'\s+','',cfg[a:b])==basearmour
 for addon,white in [('GRZ_AnimatedHexgridWhite',True),('GRZ_AnimatedHexgridBlackV2',False)]:
  _,f=read_pbo(z.read(addon+'.pbo'))
  for path,d in f.items():
   if not path.endswith('.paa'):continue
   off=struct.unpack_from('<I',d,14)[0];w,h=struct.unpack_from('<HH',d,off);length=int.from_bytes(d[off+4:off+7],'little')
   hdr=io.BytesIO();Image.new('RGBA',(w,h)).save(hdr,format='DDS',pixel_format='DXT1')
   arr=np.asarray(Image.open(io.BytesIO(hdr.getvalue()[:128]+d[off+7:off+7+length])).convert('RGB'))
   target=255 if white else 0
   assert (arr[:,:500]==target).all() and (arr[610:]==target).all(),path
print('VIP ownership/access, OTHER list routing, paid catalogue isolation, one-slot gear grant, inventory preflight, unchanged armour, all new frame base colours and thirteen-addon package passed. Static/package checks only.')
