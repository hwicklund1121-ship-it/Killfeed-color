from pathlib import Path
p=Path(__file__).resolve().parents[1] / 'GRZ_Hitmarker/gui/layouts'
# Native option selectors create their own arrow/value widgets in these value hosts.
def frame(kind,name,x,y,w,h,props='',children=''):
 return f'{kind}WidgetClass {name}\n{{\nposition {x} {y}\nsize {w} {h}\n{props}\n'+('{\n'+children+'}\n' if children else '')+'}\n'
def txt(name,label,x,y,w,h,extra=''):
 return frame('Text',name,x,y,w,h,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\nignorepointer 1\ncolor 1 1 1 1\nfont "gui/fonts/sdf_MetronLight16"\n"text valign" center\ntext "'+label+'"\n'+extra)
rows=[('Show','SHOW HITMARKERS'),('Damage','SHOW DAMAGE INFO'),('Sound','HITMARKER SOUND'),('Kills','HIGHLIGHT KILLS'),('BodyColor','HITMARKER COLOR'),('HeadColor','HEADSHOT COLOR'),('Size','HITMARKER SIZE'),('Volume','HITMARKER VOLUME')]
body=frame('Panel','Header',0,0,1,36,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\nignorepointer 1\nstyle rover_sim_colorable\ncolor 0 0 0 1')
body+=txt('Heading','GRZ HITMARKER',0,0,1,36,'"text halign" center')
for i,(name,label) in enumerate(rows):
 child=txt(name+'Label',label,0.01,0,.51,32)
 child+=frame('Frame',name+'Value',.53,0,.47,32,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\ncolor 0 0 0 0')
 body+=frame('Frame',name+'Row',0,40+i*34,1,32,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\ncolor 0 0 0 0',child)
for name,label,x,w in [('TestBody','TEST HIT',0,.31),('TestHead','TEST HEADSHOT',.345,.31),('TestKill','PREVIEW KILL',.69,.31)]:
 body+=frame('Button',name,x,322,w,34,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\ncolor 1 1 1 1',txt(name+'Label',label,0,0,1,34,'"text halign" center'))
body+=txt('PreviewLabel','PREVIEW',0.01,370,.25,28)
body+=frame('Frame','PreviewHost',.28,365,.72,92,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\ncolor 0 0 0 0')
body+=txt('AudioStatus','Headshot: standard sound (Rust clip not installed yet).',.01,465,.99,26)
(p/'options.layout').write_text(frame('Frame','GRZHitmarkerOptions',0,0,1,505,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\ncolor 0 0 0 0',body))
parts=''
for i,(x,y) in enumerate([(.28,.28),(.72,.28),(.28,.72),(.72,.72)]):
 for typ,w,h in [('Edge',.25,.085),('Stroke',.20,.035)]:
  parts+=frame('Panel',typ+str(i),x-w/2,y-h/2,w,h,'hexactpos 0\nvexactpos 0\nhexactsize 0\nvexactsize 0\nignorepointer 1\nstyle rover_sim_colorable\ncolor 1 1 1 1')
parts+=frame('Text','Damage',-.75,1.15,2.5,.75,'hexactpos 0\nvexactpos 0\nhexactsize 0\nvexactsize 0\nignorepointer 1\ncolor 1 1 1 1\nfont "gui/fonts/sdf_MetronLight24"\n"exact text" 1\n"text halign" center\n"text valign" center\ntext ""')
(p/'marker.layout').write_text(frame('Frame','GRZHitmarkerVisual',0,0,64,64,'hexactpos 1\nvexactpos 1\nhexactsize 1\nvexactsize 1\nignorepointer 1\nclipchildren 0\nvisible 0\ncolor 0 0 0 0',parts))
