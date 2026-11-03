"""Run the actual economy setup against isolated mission XML fixtures."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile
import xml.etree.ElementTree as ET
ROOT=Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--pwsh',required=True)
    args=parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='grz empty world ') as temporary:
        root=Path(temporary)
        mission=root/'mpmissions/dayzOffline.chernarusplus'
        (mission/'db').mkdir(parents=True)
        (mission/'custom').mkdir()
        (mission/'storage_1').mkdir()
        (root/'serverDZ.cfg').write_text('class Missions { class DayZ { template = "dayzOffline.chernarusplus"; }; };')
        (mission/'db/types.xml').write_text('<types><!--keep--><type name="M4A1"><nominal>20</nominal><min>5</min><lifetime>3600</lifetime></type><type name="Hat"><nominal>0</nominal></type></types>')
        (mission/'db/events.xml').write_text('<events><event name="Infected"><active>1</active><nominal>50</nominal></event><event name="Wolf"><active>1</active></event></events>')
        (mission/'custom/types.xml').write_text('<types><type name="CustomLoot"><nominal>4</nominal><min>2</min></type></types>')
        (mission/'db/globals.xml').write_text('<variables><var name="ZombieMaxCount" value="1000" type="0"/><var name="AnimalMaxCount" value="200" type="0"/><var name="Other" value="10" type="0"/></variables>')
        (mission/'economy.xml').write_text('<economy><player init="1" load="1" respawn="1" save="1"/></economy>')
        (mission/'storage_1/players.db').write_bytes(b'player fixture unchanged')
        before={p.relative_to(mission):p.read_bytes() for p in mission.rglob('*') if p.is_file()}
        env=os.environ.copy()
        for kind in ['CACHE','CONFIG','DATA']:env['XDG_'+kind+'_HOME']=str(root/kind.lower())
        command=[args.pwsh,'-NoProfile','-File',str(ROOT/'packaging/local-server/Setup-GRZEmptyWorld.ps1'),'-ServerPath',str(root)]
        result=subprocess.run(command,capture_output=True,text=True,env=env)
        assert result.returncode==0,result.stdout+result.stderr
        for file in [mission/'db/types.xml',mission/'custom/types.xml']:
            for entry in ET.parse(file).getroot():
                assert entry.findtext('nominal')=='0' and entry.findtext('min')=='0'
        for event in ET.parse(mission/'db/events.xml').getroot():assert event.findtext('active')=='0'
        assert ET.parse(mission/'db/types.xml').find('type/lifetime').text=='3600'
        globals_=ET.parse(mission/'db/globals.xml').getroot()
        assert [v.attrib['value'] for v in globals_]==['0','0','10']
        assert (mission/'storage_1/players.db').read_bytes()==before[Path('storage_1/players.db')]
        assert (mission/'economy.xml').read_bytes()==before[Path('economy.xml')]
        backups=list((root/'GRZ-EmptyWorld-Backups').glob('*/dayzOffline.chernarusplus'))
        assert len(backups)==1
        for relative in ['db/types.xml','db/events.xml','db/globals.xml','custom/types.xml']:
            assert (backups[0]/relative).read_bytes()==before[Path(relative)]
        result=subprocess.run(command,capture_output=True,text=True,env=env)
        assert result.returncode==0,result.stdout+result.stderr
        assert len(list((root/'GRZ-EmptyWorld-Backups').glob('*/dayzOffline.chernarusplus')))==1
        print('Empty-world setup passed: native/custom loot zeroed, events/AI disabled, original XML backed up, player/world persistence preserved, second run idempotent.')


if __name__=='__main__':main()
