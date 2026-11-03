from pathlib import Path
p=Path(__file__).resolve().parents[1] / 'GRZ_Hitmarker/gui/layouts'
# Native option selectors create their own arrow/value widgets in these value hosts.
def frame(kind,name,x,y,w,h,props='',children=''):
 return f'{kind}WidgetClass {name}\n{{\nposition {x} {y}\nsize {w} {h}\n{props}\n'+('{\n'+children+'}\n' if children else '')+'}\n'
def txt(name,label,x,y,w,h,extra=''):
 return frame('Text',name,x,y,w,h,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\nignorepointer 1\ncolor 1 1 1 1\nfont "gui/fonts/sdf_MetronLight16"\n"text valign" center\ntext "'+label+'"\n'+extra)
rows=[('Show','SHOW HITMARKERS'),('Damage','SHOW DAMAGE INFO'),('Sound','HITMARKER SOUND'),('Kills','HIGHLIGHT KILLS'),('BodyColor','HITMARKER COLOR'),('HeadColor','HEADSHOT COLOR'),('Size','HITMARKER SIZE'),('Volume','HITMARKER VOLUME')]
body=frame('Panel','SettingsBacking',0,0,1,366,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\nignorepointer 1\nstyle rover_sim_colorable\ncolor 0 0 0 0.45')
body+=frame('Panel','Header',0,0,1,30,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\nignorepointer 1\nstyle rover_sim_colorable\ncolor 0 0 0 1')
body+=txt('Heading','GRZ HITMARKER',0,0,1,30,'"text halign" center')
for i,(name,label) in enumerate(rows):
 child=txt(name+'Label',label,0.01,0,.51,25)
 child+=frame('Frame',name+'Value',.53,0,.47,25,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\ncolor 0 0 0 0')
 body+=frame('Frame',name+'Row',0,32+i*26,1,25,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\ncolor 0 0 0 0',child)
body+=txt('PreviewLabel','PREVIEW:',.27,250,.21,24,'"text halign" right')
for name,x,y,w,h in [('PreviewTop',.50,250,.50,1),('PreviewBottom',.50,359,.50,1),('PreviewLeft',.50,250,.003,110),('PreviewRight',.997,250,.003,110)]:
 body+=frame('Panel',name,x,y,w,h,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\nignorepointer 1\nstyle rover_sim_colorable\ncolor 0 0 0 0.9')
body+=frame('Panel','PreviewBacking',.503,251,.494,108,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\nignorepointer 1\nstyle rover_sim_colorable\ncolor 0 0 0 0.25')
body+=frame('Frame','PreviewHost',.51,256,.48,98,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\ncolor 0 0 0 0')
(p/'options.layout').write_text(frame('Frame','GRZHitmarkerOptions',0,0,1,366,'hexactpos 0\nvexactpos 1\nhexactsize 0\nvexactsize 1\ncolor 0 0 0 0',body))
parts=frame('Canvas','MarkerLines',0,0,1,1,'hexactpos 0\nvexactpos 0\nhexactsize 0\nvexactsize 0\nignorepointer 1\ncolor 1 1 1 1')
parts+=frame('Text','Damage',-.75,1.15,2.5,.75,'hexactpos 0\nvexactpos 0\nhexactsize 0\nvexactsize 0\nignorepointer 1\ncolor 1 1 1 1\nfont "gui/fonts/sdf_MetronLight24"\n"exact text" 1\n"text halign" center\n"text valign" center\ntext ""')
(p/'marker.layout').write_text(frame('Frame','GRZHitmarkerVisual',0,0,64,64,'hexactpos 1\nvexactpos 1\nhexactsize 1\nvexactsize 1\nignorepointer 1\nstyle blank\nclipchildren 0\nvisible 0\ncolor 0 0 0 0',parts))
