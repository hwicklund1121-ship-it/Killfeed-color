class GRZDuelPaint
{
 static void Line(CanvasWidget canvas, float size, float ax, float ay, float bx, float by, int color, float width = 2)
 {
  canvas.DrawLine(ax * size, ay * size, bx * size, by * size, width, color);
 }
 static void Badge(CanvasWidget canvas, int elo)
 {
  if (!canvas) return;
  float w, h; canvas.GetScreenSize(w, h);
  float s = Math.Min(w, h);
  if (s < 8) return;
  canvas.Clear();
  int color = GRZDuelRanks.Color(elo);
  int tier = GRZDuelRanks.Tier(elo);
  array<float> shield = {0.5,0.08,0.86,0.22,0.8,0.62,0.5,0.92,0.2,0.62,0.14,0.22,0.5,0.08};
  for (int i = 0; i < shield.Count() - 2; i += 2) Line(canvas,s,shield[i],shield[i+1],shield[i+2],shield[i+3],color);
  int chevrons = Math.Min(tier + 1, 4);
  for (int c = 0; c < chevrons; c++)
  {
   float y = 0.35 + c * 0.1;
   Line(canvas,s,0.32,y,0.5,y+0.11,color); Line(canvas,s,0.5,y+0.11,0.68,y,color);
  }
  if (tier >= 2)
  {
   Line(canvas,s,0.5,0.18,0.6,0.28,color); Line(canvas,s,0.6,0.28,0.5,0.38,color);
   Line(canvas,s,0.5,0.38,0.4,0.28,color); Line(canvas,s,0.4,0.28,0.5,0.18,color);
  }
  if (tier >= 3)
  {
   Line(canvas,s,0.2,0.4,0.04,0.27,color); Line(canvas,s,0.04,0.27,0.17,0.6,color);
   Line(canvas,s,0.8,0.4,0.96,0.27,color); Line(canvas,s,0.96,0.27,0.83,0.6,color);
  }
  if (tier >= 4)
  {
   Line(canvas,s,0.4,0.28,0.6,0.28,0xFFFFFFFF,1);
   Line(canvas,s,0.5,0.18,0.5,0.38,0xFFFFFFFF,1);
  }
  if (tier >= 5)
  {
   array<float> crown = {0.27,0.18,0.23,0.01,0.39,0.11,0.5,0.0,0.61,0.11,0.77,0.01,0.73,0.18};
   for (int n = 0; n < crown.Count() - 2; n += 2) Line(canvas,s,crown[n],crown[n+1],crown[n+2],crown[n+3],color);
  }
  if (tier >= 6)
  {
   for (int leaf = 0; leaf < 3; leaf++)
   {
    float ly = 0.58 + leaf * 0.09;
    Line(canvas,s,0.18,ly,0.08,ly-0.08,color); Line(canvas,s,0.82,ly,0.92,ly-0.08,color);
   }
  }
  if (tier >= 7)
  {
   for (int ray = 0; ray < 4; ray++)
   {
    float rx = 0.31 + ray * 0.125;
    Line(canvas,s,rx,0.88,rx,1,color);
   }
  }
 }
};
class GRZDuelMenu : UIScriptedMenu
{
 protected TextListboxWidget m_Players;
 protected TextListboxWidget m_Maps;
 protected ref array<string> m_PlayerIds = new array<string>;
 protected ref array<string> m_MapIds = new array<string>;
 protected string m_SelectedPlayer;
 protected string m_SelectedMap;
 protected int m_BestOf = 1;
 protected float m_Poll;
 protected int m_LastSnapshotAt;

 override Widget Init()
 {
  layoutRoot = g_Game.GetWorkspace().CreateWidgets("GRZ_Loadout/gui/layouts/duels.layout");
  m_Players = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("Players"));
  m_Maps = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("Maps"));
  ImageWidget logo = ImageWidget.Cast(layoutRoot.FindAnyWidget("Logo"));
  logo.LoadImageFile(0,"GRZ_Loadout/data/logo.paa"); logo.SetImage(0);
  logo.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND);
  GRZDuelClientBus.Changed.Insert(Receive);
  Request(); Receive();
  return layoutRoot;
 }
 static void Request()
 {
  PlayerBase p = PlayerBase.Cast(g_Game.GetPlayer());
  if (!p) return;
  ScriptRPC rpc = new ScriptRPC;
  rpc.Send(p, GRZDuelRPC.REQUEST, true, null);
 }
 protected void Text(string name, string text)
 {
  TextWidget w = TextWidget.Cast(layoutRoot.FindAnyWidget(name));
  if (w) w.SetText(text);
 }
 void Receive()
 {
  GRZDuelSnapshot data = GRZDuelClientBus.Data;
  if (!data) { Text("Status","Loading online players and duel maps..."); return; }
  m_LastSnapshotAt = GRZDuelClientBus.ReceivedAt;
  int previousPlayer = m_Players.GetSelectedRow();
  int previousMap = m_Maps.GetSelectedRow();
  if (previousPlayer >= 0 && previousPlayer < m_PlayerIds.Count()) m_SelectedPlayer = m_PlayerIds[previousPlayer];
  if (previousMap >= 0 && previousMap < m_MapIds.Count()) m_SelectedMap = m_MapIds[previousMap];
  m_Players.ClearItems(); m_Maps.ClearItems(); m_PlayerIds.Clear(); m_MapIds.Clear();
  PlayerBase self = PlayerBase.Cast(g_Game.GetPlayer());
  string myId;
  if (self && self.GetIdentity()) myId = self.GetIdentity().GetId();
  foreach (GRZDuelPlayerRow row : data.Players)
  {
   string label = row.Name + " | " + GRZDuelRanks.Name(row.Elo) + " " + row.Elo.ToString();
   if (row.Id == myId) label += " (YOU)";
   else if (!row.Available) label += " [BUSY]";
   int index = m_Players.AddItem(label,null,0);
   m_Players.SetItemColor(index,0,GRZDuelRanks.Color(row.Elo));
   m_PlayerIds.Insert(row.Id);
   if (row.Id == m_SelectedPlayer) m_Players.SelectRow(index);
  }
  foreach (GRZDuelMapRow arena : data.Maps)
  {
   string mapLabel = arena.Name;
   if (arena.Busy) mapLabel += " [IN USE]";
   int mapIndex = m_Maps.AddItem(mapLabel,null,0);
   m_MapIds.Insert(arena.Id);
   if (arena.Id == m_SelectedMap) m_Maps.SelectRow(mapIndex);
  }
  Text("MyRank",GRZDuelRanks.Name(data.Elo) + " | " + data.Elo.ToString() + " ELO");
  TextWidget rank = TextWidget.Cast(layoutRoot.FindAnyWidget("MyRank"));
  if (rank) rank.SetColor(GRZDuelRanks.Color(data.Elo));
  Text("Status",data.Status);
  UpdateDetails();
 }
 protected void UpdateDetails()
 {
  GRZDuelSnapshot data = GRZDuelClientBus.Data;
  if (!data) return;
  int playerRow = m_Players.GetSelectedRow();
  if (playerRow >= 0 && playerRow < m_PlayerIds.Count()) m_SelectedPlayer = m_PlayerIds[playerRow];
  int mapRow = m_Maps.GetSelectedRow();
  if (mapRow >= 0 && mapRow < m_MapIds.Count()) m_SelectedMap = m_MapIds[mapRow];
  GRZDuelPlayerRow selected;
  foreach (GRZDuelPlayerRow player : data.Players) if (player.Id == m_SelectedPlayer) { selected = player; break; }
  bool available = false;
  string selectedLabel = "Select an opponent";
  if (selected)
  {
   selectedLabel = selected.Name + " | " + GRZDuelRanks.Name(selected.Elo) + " " + selected.Elo.ToString();
   Text("SelectedGun", "CURRENT GUN: " + selected.Weapon);
   PlayerBase me = PlayerBase.Cast(g_Game.GetPlayer());
   available = selected.Available && me && me.GetIdentity() && selected.Id != me.GetIdentity().GetId();
   GRZDuelPaint.Badge(CanvasWidget.Cast(layoutRoot.FindAnyWidget("OpponentBadge")),selected.Elo);
   TextWidget opponentRank = TextWidget.Cast(layoutRoot.FindAnyWidget("SelectedPlayer"));
   if (opponentRank) opponentRank.SetColor(GRZDuelRanks.Color(selected.Elo));
  }
  Text("SelectedPlayer",selectedLabel);
  bool mapAvailable;
  foreach (GRZDuelMapRow mapEntry : data.Maps) if (mapEntry.Id == m_SelectedMap && !mapEntry.Busy) mapAvailable = true;
  layoutRoot.FindAnyWidget("SendChallenge").Enable(available && mapAvailable && data.PendingId == "" && !data.InMatch);
  layoutRoot.FindAnyWidget("Accept").Show(data.PendingId != "" && data.Incoming);
  layoutRoot.FindAnyWidget("Decline").Show(data.PendingId != "");
  layoutRoot.FindAnyWidget("Forfeit").Show(data.InMatch);
  if (data.Incoming) Text("DeclineLabel","DECLINE"); else Text("DeclineLabel","CANCEL");
  string pending;
  if (data.PendingId != "")
  {
   int left = Math.Max(0,data.PendingSeconds - (g_Game.GetTime() - m_LastSnapshotAt) / 1000);
   string direction = "Challenge sent to "; if (data.Incoming) direction = "CHALLENGE FROM ";
   pending = direction + data.PendingName + " | " + data.PendingMap + " | BO" + data.PendingBestOf.ToString() + " | " + left.ToString() + "s";
  }
  else if (data.InMatch) pending = "DUEL: " + data.MapName + " | " + data.ScoreSelf.ToString() + " - " + data.ScoreOpponent.ToString();
  Text("Pending",pending);
  GRZDuelPaint.Badge(CanvasWidget.Cast(layoutRoot.FindAnyWidget("MyBadge")),data.Elo);
  array<string> buttons = {"Best1Fill","Best3Fill","Best5Fill"};
  array<int> values = {1,3,5};
  for (int i = 0; i < buttons.Count(); i++)
  {
   int tint = 0xFF202026;
   if (m_BestOf == values[i]) tint = 0xFF555565;
   layoutRoot.FindAnyWidget(buttons[i]).SetColor(tint);
  }
 }
 override void Update(float timeslice)
 {
  super.Update(timeslice);
  UpdateDetails();
  m_Poll += timeslice;
  if (m_Poll >= 2.5) { m_Poll = 0; Request(); }
 }
 override bool OnClick(Widget w, int x, int y, int button)
 {
  if (button != MouseState.LEFT) return super.OnClick(w,x,y,button);
  string name = w.GetName();
  if (name == "Close") { Close(); return true; }
  if (name == "Best1") { m_BestOf = 1; return true; }
  if (name == "Best3") { m_BestOf = 3; return true; }
  if (name == "Best5") { m_BestOf = 5; return true; }
  PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
  if (!player) return true;
  if (name == "SendChallenge")
  {
   UpdateDetails();
   ScriptRPC send = new ScriptRPC;
   send.Write(m_SelectedPlayer); send.Write(m_SelectedMap); send.Write(m_BestOf);
   send.Send(player,GRZDuelRPC.CHALLENGE,true,null);
   return true;
  }
  if (name == "Accept" || name == "Decline")
  {
   if (!GRZDuelClientBus.Data || GRZDuelClientBus.Data.PendingId == "") return true;
   ScriptRPC answer = new ScriptRPC;
   answer.Write(GRZDuelClientBus.Data.PendingId); answer.Write(name == "Accept");
   answer.Send(player,GRZDuelRPC.ANSWER,true,null);
   return true;
  }
  if (name == "Forfeit")
  {
   ScriptRPC leave = new ScriptRPC;
   leave.Send(player,GRZDuelRPC.LEAVE,true,null);
   Close(); return true;
  }
  return super.OnClick(w,x,y,button);
 }
 override void OnHide()
 {
  GRZDuelClientBus.Changed.Remove(Receive);
  super.OnHide();
 }
 void ~GRZDuelMenu()
 {
  GRZDuelClientBus.Changed.Remove(Receive);
 }
 override bool OnKeyDown(Widget w, int x, int y, int key)
 {
  if (key == KeyCode.KC_ESCAPE) { Close(); return true; }
  return super.OnKeyDown(w,x,y,key);
 }
};
modded class MissionBase
{
 override UIScriptedMenu CreateScriptedMenu(int id)
 {
  if (id == GRZDuelRPC.MENU)
  {
   GRZDuelMenu menu = new GRZDuelMenu;
   menu.SetID(id); return menu;
  }
  return super.CreateScriptedMenu(id);
 }
};
modded class InGameMenu
{
 void GRZDuelAutoRespawn()
 {
  MissionGameplay mission = MissionGameplay.Cast(g_Game.GetMission());
  PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
  if (!mission || mission.IsPlayerRespawning() || (player && player.IsAlive()) || !GRZDuelClientBus.Data || !GRZDuelClientBus.Data.AutoRespawn) return;
  GameRespawn(false);
 }
};
modded class MissionGameplay
{
 protected Widget m_GRZDuelHud;
 protected int m_GRZDuelNoticeUntil;
 protected string m_GRZDuelPrompted;
 protected bool m_GRZDuelWasActive;
 protected bool m_GRZDuelFireBlocked;
 protected float m_GRZDuelHeartbeat;
 protected int m_GRZDuelDeadSince = -1;
 override void OnInit()
 {
  super.OnInit();
  GRZDuelClientBus.Notice.Insert(GRZDuelNotice);
 }
 void GRZDuelNotice(string message)
 {
  if (!m_GRZDuelHud) m_GRZDuelHud = g_Game.GetWorkspace().CreateWidgets("GRZ_Loadout/gui/layouts/duel-notice.layout");
  if (m_GRZDuelHud)
  {
   TextWidget text = TextWidget.Cast(m_GRZDuelHud.FindAnyWidget("DuelMessage"));
   if (text) text.SetText(message);
   m_GRZDuelNoticeUntil = g_Game.GetTime() + 5000;
   m_GRZDuelHud.Show(true);
  }
 }
 override void OnUpdate(float timeslice)
 {
  super.OnUpdate(timeslice);
  if (m_GRZDuelHud && g_Game.GetTime() >= m_GRZDuelNoticeUntil) m_GRZDuelHud.Show(false);
  GRZDuelSnapshot data = GRZDuelClientBus.Data;
  if (!data || !g_Game.IsAppActive()) return;
  bool blockFire = data.InMatch && data.Phase != 2;
  if (blockFire != m_GRZDuelFireBlocked)
  {
   UAInput fire = GetUApi().GetInputByName("UAFire");
   if (fire) fire.ForceDisable(blockFire);
   m_GRZDuelFireBlocked = blockFire;
  }
  if (data.InMatch && !m_GRZDuelWasActive)
  {
   UIScriptedMenu opened = g_Game.GetUIManager().GetMenu();
   if (opened && opened.GetID() == GRZDuelRPC.MENU) opened.Close();
  }
  m_GRZDuelWasActive = data.InMatch;
  if (m_GRZDuelHud)
  {
   Widget noticeBox = m_GRZDuelHud.FindAnyWidget("NoticeBox");
   Widget scoreBox = m_GRZDuelHud.FindAnyWidget("ScoreBox");
   if (noticeBox) noticeBox.Show(g_Game.GetTime() < m_GRZDuelNoticeUntil);
   if (scoreBox) scoreBox.Show(data.InMatch);
   TextWidget score = TextWidget.Cast(m_GRZDuelHud.FindAnyWidget("DuelScore"));
   if (score) score.SetText(data.MapName + " | YOU " + data.ScoreSelf.ToString() + " - " + data.ScoreOpponent.ToString() + " " + data.Opponent);
   m_GRZDuelHud.Show(data.InMatch || g_Game.GetTime() < m_GRZDuelNoticeUntil);
  }
  PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
  m_GRZDuelHeartbeat += timeslice;
  if (m_GRZDuelHeartbeat >= 5 && player && player.IsAlive() && (data.InMatch || data.AutoRespawn || data.PendingId != ""))
  {
   m_GRZDuelHeartbeat = 0;
   GRZDuelMenu.Request();
  }
  if (data.AutoRespawn && (!player || !player.IsAlive()) && !IsPlayerRespawning())
  {
   if (m_GRZDuelDeadSince < 0) m_GRZDuelDeadSince = g_Game.GetTime();
   if (g_Game.GetTime() - m_GRZDuelDeadSince >= 1000)
   {
    UIScriptedMenu current = g_Game.GetUIManager().GetMenu();
    if (current && (current.GetID() == GRZDuelRPC.MENU || current.GetID() == GRZLeaderboardRPC.MENU || current.GetID() == GRZLoadoutRPC.MENU)) current.Close();
    if (!g_Game.GetUIManager().GetMenu() && !g_Game.GetUIManager().IsDialogVisible())
    {
     InGameMenu respawnMenu = InGameMenu.Cast(g_Game.GetUIManager().EnterScriptedMenu(MENU_INGAME,null));
     if (respawnMenu) respawnMenu.GRZDuelAutoRespawn();
    }
   }
   return;
  }
  m_GRZDuelDeadSince = -1;
  if (data.Incoming && data.PendingId != "" && data.PendingId != m_GRZDuelPrompted && player && player.IsAlive() && !g_Game.GetUIManager().GetMenu() && !g_Game.GetUIManager().IsDialogVisible())
  {
   m_GRZDuelPrompted = data.PendingId;
   g_Game.GetUIManager().EnterScriptedMenu(GRZDuelRPC.MENU,null);
  }
 }
 override void OnMissionFinish()
 {
  if (m_GRZDuelFireBlocked)
  {
   UAInput fire = GetUApi().GetInputByName("UAFire");
   if (fire) fire.ForceDisable(false);
   m_GRZDuelFireBlocked = false;
  }
  GRZDuelClientBus.Notice.Remove(GRZDuelNotice);
  GRZDuelClientBus.Data = null;
  if (m_GRZDuelHud) { m_GRZDuelHud.Unlink(); m_GRZDuelHud = null; }
  super.OnMissionFinish();
 }
};
modded class MissionServer
{
 override void OnMissionStart()
 {
  super.OnMissionStart();
  GRZDuelServer.Init();
 }
 override void OnMissionFinish()
 {
  GRZDuelServer.Stop();
  super.OnMissionFinish();
 }
};
