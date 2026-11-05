"""Convert supplied M100 part PNGs into native-size colour and normal PAA textures."""
import sys,io,json,hashlib,struct,argparse
from pathlib import Path
from zipfile import ZipFile,ZIP_DEFLATED
from PIL import Image
sys.path.insert(0,'/tmp/grz-paa-deps')
from build_status_textures import encode_paa
import lzokay
root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('part',choices=['barrel','bolt','grip','handgrip','muzzle','stock','pants','shoes','hexgrid'])
parser.add_argument('source_zip',type=Path)
parser.add_argument('--addon', choices=['GRZ_M100','GRZ_XimsKit'],default='GRZ_M100')
args=parser.parse_args()
archive_name = 'RVK-M100-Textures' if args.addon=='GRZ_M100' else 'XIMS-Kit-Textures'
manifest_name = 'RVK-M100-TEXTURES-MANIFEST' if args.addon=='GRZ_M100' else 'XIMS-KIT-TEXTURES-MANIFEST'
target=root/'packaging/customs'/args.addon/'data';target.mkdir(parents=True,exist_ok=True)
manifest_path=root/'downloads'/(manifest_name+'.json')
manifest=json.loads(manifest_path.read_text()) if manifest_path.exists() else {}
with ZipFile(args.source_zip) as archive:
    names=[n for n in archive.namelist() if n.lower().endswith('.png')]
    assert names
    for name in names:
        source=archive.read(name);im=Image.open(io.BytesIO(source)).convert('RGBA')
        w,h=im.size;assert w==h and w&(w-1)==0
        normal='normal' in name.lower();fmt='DXT5' if normal else 'DXT1'
        filename=args.part+('_nohq.paa' if normal else '_co.paa')
        paa=encode_paa(im,fmt)
        at=struct.unpack_from('<I',paa,14)[0]
        stored_w,stored_h=struct.unpack_from('<HH',paa,at)
        size=int.from_bytes(paa[at+4:at+7],'little');blocks=paa[at+7:at+7+size]
        expected=w*h//(1 if normal else 2)
        if stored_w&0x8000:blocks=lzokay.decompress(blocks,expected)
        assert len(blocks)==expected and (stored_w&0x7fff,stored_h)==(w,h)
        dds=io.BytesIO();im.save(dds,format='DDS',pixel_format=fmt)
        assert blocks==dds.getvalue()[128:]
        decoded=Image.open(io.BytesIO(dds.getvalue()[:128]+blocks));decoded.load()
        assert decoded.size==im.size
        (target/filename).write_bytes(paa)
        manifest[filename]=dict(source=name,size=[w,h],format=fmt,source_sha256=hashlib.sha256(source).hexdigest(),paa_sha256=hashlib.sha256(paa).hexdigest(),paa_bytes=len(paa))
manifest_path.write_text(json.dumps(manifest,indent=2)+'\n')
with ZipFile(root/'downloads'/(archive_name+'.zip'),'w',ZIP_DEFLATED) as archive:
    for p in sorted(target.glob('*.paa')):archive.write(p,p.name)
    archive.writestr('MANIFEST.json',json.dumps(manifest,indent=2)+'\n')
print(args.part+': PAA conversion and decoded DDS block verification passed at original resolution.')
