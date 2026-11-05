"""Build the GRZ loadout addon using the supplied transparent logo ZIP."""
import argparse
import hashlib
import io
import json
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
from PIL import Image
from build_pbo import build
from build_status_textures import encode_paa
root = Path(__file__).resolve().parents[1]
source = root / 'GRZ_Loadout'

def widget(kind, name, x, y, w, h, props='', children=''):
    return f'''{kind} {name}
{{
 position {x} {y}
 size {w} {h}
 hexactpos 0
 vexactpos 0
 hexactsize 0
 vexactsize 0
 {props}
'''+ (' {\n'+children+' }\n' if children else '')+'}\n'

def panel(name,x,y,w,h,color):
    return widget('PanelWidgetClass',name,x,y,w,h,f'ignorepointer 1\n style rover_sim_colorable\n color {color}')

def text(name,value,x,y,w,h,size=24,color='1 1 1 1',center=False):
    return widget('TextWidgetClass',name,x,y,w,h,
                  f'ignorepointer 1\n color {color}\n font "gui/fonts/sdf_MetronLight{size}"\n "exact text" 1\n "text valign" center\n "text halign" '+('center' if center else 'left')+f'\n text "{value}"')

def button(name,label,x,y,w,h,white=False):
    fill = '0.96 0.96 0.96 1' if white else '0.085 0.085 0.095 1'
    ink = '0.025 0.025 0.03 1' if white else '1 1 1 1'
    children = panel(name+'Fill',0,0,1,1,fill)+text(name+'Label',label,0,0,1,1,24,ink,True)
    return widget('ButtonWidgetClass',name,x,y,w,h,'color 1 1 1 1',children)

def layout():
    children = panel('MenuBackground',0,0,1,1,'0.025 0.025 0.03 0.98')
    children += panel('AccentTop',0,0,1,.004,'1 1 1 1')
    children += widget('ImageWidgetClass','Logo',.027,.022,.245,.13,'ignorepointer 1\n color 1 1 1 1\n mode blend\n stretch 1')
    children += text('Heading','BUILD YOUR LOADOUT',.31,.053,.55,.06,24,'0.75 0.75 0.78 1')
    children += button('Close','X',.935,.035,.04,.055)
    children += button('LoadoutTab','LOADOUT',.027,.177,.18,.065,True)
    children += widget('ButtonWidgetClass','ClothingTab',.22,.177,.18,.065,'color 1 1 1 1',panel('ClothingFill',0,0,1,1,'0.06 0.06 0.07 1')+text('ClothingLabel','CLOTHING - SOON',0,0,1,1,16,'0.42 0.42 0.44 1',True))
    children += panel('Divider',.027,.268,.946,.0015,'0.22 0.22 0.24 1')
    for name,label,x in [('Calibre20','20MM',.29),('Calibre50','.50 CAL',.445),('Calibre408','.408',.60)]:
        children += button(name,label,x,.29,.14,.06)
        children += panel(name+'Line',x,.355,.14,.003,'1 1 1 1')
    children += panel('PreviewBackground',.075,.39,.85,.315,'0.055 0.055 0.065 1')
    children += widget('ItemPreviewWidgetClass','GunPreview',.115,.39,.77,.315,'ignorepointer 1\n mode blend\n color 1 1 1 1')
    children += button('GunPrevious','<',.027,.51,.043,.075)
    children += button('GunNext','>',.93,.51,.043,.075)
    children += text('GunName','SELECT A GUN',.15,.715,.7,.05,24,center=True)
    children += text('GunCount','0 / 0',.43,.77,.14,.028,16,'0.6 0.6 0.64 1',True)
    children += text('WrapHeading','GUN WRAP',.027,.811,.16,.045,16,'0.65 0.65 0.69 1')
    children += button('WrapPrevious','<',.19,.807,.04,.05)
    children += text('WrapName','No wrap',.24,.808,.42,.05,16,center=True)
    children += button('WrapNext','>',.67,.807,.04,.05)
    children += button('Save','SAVE LOADOUT',.75,.812,.223,.10,True)
    children += text('Status','',.027,.935,.946,.04,16,'0.75 0.75 0.78 1')
    content = widget('FrameWidgetClass','MenuPanel',.07,.065,.86,.87,'color 0 0 0 0',children)
    return widget('FrameWidgetClass','GRZLoadoutRoot',0,0,1,1,'color 0 0 0 0',content)

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('logo_zip',type=Path)
args = parser.parse_args()
(source / 'gui/layouts/loadout.layout').write_text(layout())
with ZipFile(args.logo_zip) as archive:
    names = [n for n in archive.namelist() if n.lower().endswith('.png')]
    if len(names)!=1:
        raise SystemExit('Expected one logo PNG')
    original = archive.read(names[0])
    logo = Image.open(io.BytesIO(original)).convert('RGBA')
(source / 'data/logo.paa').write_bytes(encode_paa(logo.resize((2048,512),Image.Resampling.LANCZOS), alpha_flags=True))
pbo = build(source)
(root / 'downloads/GRZ_Loadout.pbo').write_bytes(pbo)
notes = '''GRZ Loadout 0.1.3 — Made By Blake

INSTALL
Copy GRZ_Loadout.pbo into @GRZ_Local/Addons on BOTH game and server.
Restart both. Enable @GRZ_Local in the client launcher. Press comma (,).
The key is rebindable in DayZ Controls > GRZ Loadout.

FEATURES
Large dark menu, transparent GunRunnerZ logo at top left, white accents.
Only bolt-action rifle config families are included; semi-auto guns are excluded.
AS50 and Barrett families excluded by class, display name and model path.
The SFG permanent suppressor is explicitly attached in the local preview and server grant.
If the SFG suppressor cannot be attached, Save fails before removing existing gear.
Compatible scope is selected from the gun mount slots and included in preview and equipped gun.
Magazine-fed guns also receive six full spare magazines in cargo, plus the loaded magazine on the gun.
Permanent ammo-count and inventory-replacement captions removed; operational status/errors remain.
Loadout tab: 20mm / .50 cal / .408, previous/next arrows, local 3D preview.
Gun-wrap arrows include None and installed GhillieAtt_ColorBase subclasses.
Save replaces EVERY carried gun and ALL magazines/ammo, including cargo.
New gun is loaded, compatible scope and chosen wrap attached, plus TEN FULL loose ammo stacks.
Magazine-fed guns also get SIX full spare magazines.
Non-weapon items and clothing are kept. Items attached to removed guns are removed with those guns.
Clothing tab is a disabled placeholder for later work.

SERVER
First menu request creates profiles/GRZ_Loadout/config.json using your actual -profiles path.
Default AutoDiscover=true finds scope=2 bolt-action rifles from their compatible ammo configs.
Calibre names supported: VULCAN/20mm/20x102/20x110/20x138, 50CAL/50BMG/12.7x99/12.7x108, 408.
Matching checks ammo bullet class, ammo item class and display name; spaces, underscores and hyphens are normalized.
Server console logs the number of guns and wrap options found.
Custom configurations using different ammo names can be added to Guns in config.json:
{"Type":"YOUR_WEAPON_CLASS","Name":"Display name","Ammo":"LOOSE_AMMO_PILE_CLASS","MagazineType":"DETACHABLE_MAG_CLASS_OR_EMPTY","Category":0}
Category: 0=20mm, 1=.50 cal, 2=.408. Ammo must be the CfgMagazines ammo pile, not CfgAmmo bullet.
Restart server after configuration changes. Explicit entries take priority over discovery.
AmmoStacks defaults to 10. SaveCooldownSeconds defaults to 5.
All players can use the menu; server validates the requested classes against its catalogue.

SAVE BEHAVIOUR
Requires conscious living player, lowered gun, outside vehicles; non-weapon hand item must be put away.
Gun, attached wrap, chamber and ten ammo stacks are staged and validated before changing old gear.
Old guns/ammo temporarily move to an inaccessible staging location during the synchronous swap.
New gun goes into hands; all ten stacks must fit in clothing/backpack cargo.
If any move fails, new items are deleted and original gear locations are restored.
If another mod prevents restoration, preserved old items fall beside the owner and a script error is logged.
Only after all new items fit are the old guns/ammo deleted.
No persistent saved choices or automatic grants on respawn yet; Save equips the current selection.
Preview entities are local-only and deleted on change/close. No hit/damage/recoil callbacks changed.

VALIDATION
Static package/API checks only. Enforce compilation, preview sizing, live inventory rollback and multiplayer sync require testing in DayZ 1.30.
Unsigned development build; sign with your own server key if required.
'''
(root / 'LOADOUT.md').write_text(notes)
with ZipFile(root / 'downloads/GRZ_Loadout.zip','w',ZIP_DEFLATED) as archive:
    archive.writestr('GRZ_Loadout.pbo',pbo)
    archive.writestr('INSTALL.txt',notes)
manifest=dict(version='0.1.3',ammo_stacks=10,open_key='comma',replace='all carried guns and ammo',logo_original_size=logo.size,logo_source_sha256=hashlib.sha256(original).hexdigest(),pbo_sha256=hashlib.sha256(pbo).hexdigest())
(root / 'downloads/GRZ-LOADOUT-MANIFEST.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Built GRZ_Loadout.pbo:',len(pbo),'bytes')
