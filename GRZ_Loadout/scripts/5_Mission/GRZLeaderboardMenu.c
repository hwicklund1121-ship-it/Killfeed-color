class GRZLeaderboardMenu : UIScriptedMenu
{
 override void OnShow()
 {
  super.OnShow();
  g_Game.GetMission().AddActiveInputExcludes({"menu"});
 }
 override bool UseMouse() { return true; }
 override bool UseKeyboard() { return true; }
 override bool UseGamepad() { return true; }


 protected int m_Page;
 protected int m_Sort;
 protected int m_LastSent = -10000;
 protected float m_Refresh;
 override Widget Init()
 {
  layoutRoot = g_Game.GetWorkspace().CreateWidgets("GRZ_Loadout/gui/layouts/leaderboard.layout");
  ImageWidget logo = ImageWidget.Cast(layoutRoot.FindAnyWidget("Logo"));
  logo.LoadImageFile(0,"GRZ_Loadout/data/logo.paa"); logo.SetImage(0);
  logo.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND);
  GRZLeaderboardBus.Changed.Insert(Receive);
  if (GRZLeaderboardBus.Data) Receive();
  Fetch(m_Page,m_Sort);
  return layoutRoot;
 }
 protected void Text(string name, string value)
 {
  TextWidget w = TextWidget.Cast(layoutRoot.FindAnyWidget(name));
  if (w) w.SetText(value);
 }
 protected bool Fetch(int page, int sort)
 {
  PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
  if (!player || g_Game.GetTime() - m_LastSent < 1100) return false;
  m_LastSent = g_Game.GetTime(); m_Page = Math.Max(0,page); m_Sort = sort;
  ScriptRPC rpc = new ScriptRPC;
  rpc.Write(m_Page); rpc.Write(m_Sort); rpc.Send(player,GRZLeaderboardRPC.REQUEST,true,null);
  return true;
 }
 void Receive()
 {
  GRZLeaderboardPage data = GRZLeaderboardBus.Data;
  if (!data) return;
  m_Page = data.Page; m_Sort = data.Sort;
  for (int i = 0; i < 20; i++)
  {
   string suffix = i.ToString();
   Widget line = layoutRoot.FindAnyWidget("Row" + suffix);
   line.Show(i < data.Rows.Count());
   if (i >= data.Rows.Count()) continue;
   GRZLeaderboardRow row = data.Rows[i];
   Text("Position" + suffix,(data.Page * 20 + i + 1).ToString());
   Text("Name" + suffix,row.Name);
   Text("Kills" + suffix,row.Kills.ToString()); Text("Deaths" + suffix,row.Deaths.ToString());
   Text("KD" + suffix,GRZLeaderboardBus.Ratio(row.Kills,row.Deaths)); Text("Elo" + suffix,row.Elo.ToString());
   TextWidget elo = TextWidget.Cast(layoutRoot.FindAnyWidget("Elo" + suffix));
   if (elo) elo.SetColor(GRZDuelRanks.Color(row.Elo));
   GRZDuelPaint.Badge(CanvasWidget.Cast(layoutRoot.FindAnyWidget("Badge" + suffix)),row.Elo);
  }
  int pages = Math.Max(1,(data.Total + 19) / 20);
  Text("Page", "PAGE " + (data.Page + 1).ToString() + " / " + pages.ToString() + " | " + data.Total.ToString() + " PLAYERS");
  Text("Status",data.Status);
  Text("MyRank",GRZDuelRanks.Name(data.MyElo) + " | " + data.MyElo.ToString() + " ELO");
  TextWidget own = TextWidget.Cast(layoutRoot.FindAnyWidget("MyRank"));
  if (own) own.SetColor(GRZDuelRanks.Color(data.MyElo));
 }
 override void Update(float timeslice)
 {
  super.Update(timeslice);
  GRZLeaderboardPage data = GRZLeaderboardBus.Data;
  bool ready = g_Game.GetTime() - m_LastSent >= 1100;
  layoutRoot.FindAnyWidget("Previous").Enable(ready && data && data.Page > 0);
  layoutRoot.FindAnyWidget("Next").Enable(ready && data && (data.Page + 1) * 20 < data.Total);
  array<string> sorts = {"SortKills","SortKD","SortElo"};
  for (int s = 0; s < sorts.Count(); s++)
  {
   layoutRoot.FindAnyWidget(sorts[s]).Enable(ready);
   int color = 0xFF202026;
   if (s == m_Sort) color = 0xFF555565;
   layoutRoot.FindAnyWidget(sorts[s] + "Fill").SetColor(color);
  }
  if (data)
  {
   GRZDuelPaint.Badge(CanvasWidget.Cast(layoutRoot.FindAnyWidget("MyBadge")),data.MyElo);
   for (int i = 0; i < data.Rows.Count(); i++) GRZDuelPaint.Badge(CanvasWidget.Cast(layoutRoot.FindAnyWidget("Badge" + i.ToString())),data.Rows[i].Elo);
  }
  m_Refresh += timeslice;
  if (m_Refresh >= 5) { m_Refresh = 0; Fetch(m_Page,m_Sort); }
 }
 override bool OnClick(Widget w, int x, int y, int button)
 {
  if (button != MouseState.LEFT) return super.OnClick(w,x,y,button);
  string name = w.GetName();
  if (name == "Close") { Close(); return true; }
  if (name == "Previous") { Fetch(m_Page - 1,m_Sort); return true; }
  if (name == "Next") { Fetch(m_Page + 1,m_Sort); return true; }
  if (name == "SortKills") { Fetch(0,0); return true; }
  if (name == "SortKD") { Fetch(0,1); return true; }
  if (name == "SortElo") { Fetch(0,2); return true; }
  return super.OnClick(w,x,y,button);
 }
 override void OnHide()
 {
  g_Game.GetMission().RemoveActiveInputExcludes({"menu"}, true);
  GRZLeaderboardBus.Changed.Remove(Receive);
  super.OnHide();
 }
 void ~GRZLeaderboardMenu() { GRZLeaderboardBus.Changed.Remove(Receive); }
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
  if (id == GRZLeaderboardRPC.MENU)
  {
   GRZLeaderboardMenu menu = new GRZLeaderboardMenu;
   menu.SetID(id); return menu;
  }
  return super.CreateScriptedMenu(id);
 }
};
modded class MissionGameplay
{
 override void OnUpdate(float timeslice)
 {
  super.OnUpdate(timeslice);
  if (!g_Game.IsAppActive() || g_Game.GetUIManager().GetMenu() || g_Game.GetUIManager().IsDialogVisible()) return;
  PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
  if (!player || !player.IsAlive() || player.IsUnconscious()) return;
  UAInput input = GetUApi().GetInputByName("UAGRZLeaderboard");
  if (input && input.LocalPress()) g_Game.GetUIManager().EnterScriptedMenu(GRZLeaderboardRPC.MENU,null);
 }
 override void OnMissionFinish()
 {
  GRZLeaderboardBus.Data = null;
  super.OnMissionFinish();
 }
};
