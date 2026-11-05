"""Generate native duel and leaderboard layouts and a rank colour reference."""
from pathlib import Path

root=Path(__file__).resolve().parents[1]
out=root/'GRZ_Loadout/gui/layouts'
def widget(kind,name,x,y,w,h,props='',children=''):
 return f'{kind} {name}\n{{\n position {x} {y}\n size {w} {h}\n hexactpos 0\n vexactpos 0\n hexactsize 0\n vexactsize 0\n {props}\n'+(' {\n'+children+' }\n' if children else '')+'}\n'
def panel(name,x,y,w,h,color):
 return widget('ImageWidgetClass',name,x,y,w,h,f'ignorepointer 1\n mode blend\n stretch 1\n color {color}')
def text(name,value,x,y,w,h,size=20,color='1 1 1 1',center=False):
 if size == 20: size = 24
 return widget('TextWidgetClass',name,x,y,w,h,f'ignorepointer 1\n color {color}\n font "gui/fonts/sdf_MetronLight{size}"\n "exact text" 1\n "text valign" center\n "text halign" '+('center' if center else 'left')+f'\n text "{value}"')
def canvas(name,x,y,w,h):
 return widget('CanvasWidgetClass',name,x,y,w,h,'ignorepointer 1\n color 1 1 1 1')
def button(name,label,x,y,w,h):
 return widget('ButtonWidgetClass',name,x,y,w,h,'color 1 1 1 1',panel(name+'Fill',0,0,1,1,'0.09 0.09 0.11 1')+text(name+'Label',label,0,0,1,1,20,center=True))
def base(title):
 return panel('Background',0,0,1,1,'0 0 0 0.8')+panel('WhiteAccent',0,0,1,.004,'1 1 1 1')+widget('ImageWidgetClass','Logo',.025,.02,.245,.105,'ignorepointer 1\n color 1 1 1 1\n mode blend\n stretch 1')+text('Title',title,.33,.04,.56,.055,24)+button('Close','X',.94,.025,.04,.055)
def frame(name,children):
 return widget('FrameWidgetClass',name,0,0,1,1,'color 0 0 0 0',widget('FrameWidgetClass','MenuPanel',.07,.065,.86,.87,'color 0 0 0 0',children))
c=base('CHALLENGE A PLAYER')
c+=canvas('MyBadge',.03,.14,.065,.095)+text('MyRank','BRONZE | 800 ELO',.105,.15,.37,.06)
c+=text('MatchType','MATCH LENGTH',.52,.15,.2,.04,16)
for name,label,x in [('Best1','BEST OF 1',.52),('Best3','BEST OF 3',.675),('Best5','BEST OF 5',.83)]:c+=button(name,label,x,.2,.145,.055)
c+=text('PlayersTitle','ONLINE PLAYERS',.03,.27,.43,.04,20)+text('MapsTitle','DUEL MAPS',.52,.27,.45,.04,20)
c+=panel('PlayersBack',.03,.32,.45,.34,'0.07 0.07 0.08 1')+widget('TextListboxWidgetClass','Players',.03,.32,.45,.34,'color 1 1 1 1\n font "gui/fonts/sdf_MetronLight16"')
c+=panel('MapsBack',.52,.32,.45,.34,'0.07 0.07 0.08 1')+widget('TextListboxWidgetClass','Maps',.52,.32,.45,.34,'color 1 1 1 1\n font "gui/fonts/sdf_MetronLight24"')
c+=canvas('OpponentBadge',.03,.675,.065,.09)+text('SelectedPlayer','Select an opponent',.105,.68,.86,.04,20)+text('SelectedGun','',.105,.725,.86,.03,16,'0.7 0.7 0.74 1')
c+=text('Pending','',.03,.78,.94,.04,20)
c+=button('Accept','ACCEPT',.53,.835,.2,.055)+button('Decline','DECLINE',.75,.835,.22,.055)
c+=button('Forfeit','FORFEIT (-ELO)',.03,.9,.24,.06)+button('SendChallenge','SEND CHALLENGE',.7,.9,.27,.06)
c+=text('Status','',.03,.965,.94,.03,16,'0.75 0.75 0.8 1')
(out/'duels.layout').write_text(frame('GRZDuelsRoot',c))
notice=panel('NoticeBackground',0,0,1,1,'0.03 0.03 0.04 0.94')+text('DuelMessage','',.02,.05,.96,.9,24,center=True)
score=panel('ScoreBackground',0,0,1,1,'0.03 0.03 0.04 0.9')+text('DuelScore','',.02,.05,.96,.9,20,center=True)
(out/'duel-notice.layout').write_text(widget('FrameWidgetClass','DuelHud',.22,.13,.56,.15,'ignorepointer 1\n color 0 0 0 0',widget('FrameWidgetClass','NoticeBox',0,0,1,.6,'ignorepointer 1\n color 0 0 0 0',notice)+widget('FrameWidgetClass','ScoreBox',0,.67,1,.33,'ignorepointer 1\n color 0 0 0 0',score)))
c=base('SERVER LEADERBOARD')
c+=canvas('MyBadge',.03,.13,.055,.07)+text('MyRank','BRONZE | 800 ELO',.095,.14,.37,.055)
for name,label,x in [('SortKills','KILLS',.52),('SortKD','K/D',.675),('SortElo','ELO',.83)]:c+=button(name,label,x,.145,.145,.055)
for name,label,x,w in [('RankHeading','#',.015,.03),('NameHeading','PLAYER',.115,.37),('KillsHeading','KILLS',.53,.08),('DeathsHeading','DEATHS',.63,.08),('KDHeading','K/D',.73,.08),('EloHeading','ELO',.85,.1)]:c+=text(name,label,x,.23,w,.035,16)
for i in range(20):
 row=panel('RowBackground'+str(i),0,0,1,1,'0.055 0.055 0.065 1' if i%2==0 else '0.035 0.035 0.045 1')
 row+=text('Position'+str(i),'',.015,0,.03,1,16)+canvas('Badge'+str(i),.055,.02,.04,.96)
 for name,x,w in [('Name',.115,.37),('Kills',.53,.08),('Deaths',.63,.08),('KD',.73,.08),('Elo',.85,.1)]:row+=text(name+str(i),'',x,0,w,1,16)
 c+=widget('FrameWidgetClass','Row'+str(i),.01,.28+i*.0285,.98,.027,'color 0 0 0 0',row)
c+=button('Previous','<',.03,.89,.06,.06)+text('Page','Loading...',.12,.89,.76,.06,16,center=True)+button('Next','>',.91,.89,.06,.06)
c+=text('Status','',.03,.965,.94,.03,16,'0.75 0.75 0.8 1')
(out/'leaderboard.layout').write_text(frame('GRZLeaderboardRoot',c))
print('Built duel, notification and 20-row paginated leaderboard layouts.')
