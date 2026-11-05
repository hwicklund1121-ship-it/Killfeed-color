class GRZRotationClient
{
 static ref GRZRotationSnapshot Data;
 static int ReceivedAt;
 static int AutoPopupRound = -1;
 static bool PendingMenu;
 static bool ManualRequest;
 static ref EffectSound Chime;
 static ref ScriptInvoker Updated = new ScriptInvoker;
 static Widget Notification;
 static int NoticeUntil;

 static int Remaining()
 {
  if (!Data)
   return 0;
  return Math.Max(0, Data.Remaining - (g_Game.GetTime() - ReceivedAt) / 1000);
 }

 static string Clock(int seconds)
 {
  int minutes = seconds / 60;
  int remainder = seconds % 60;
  return minutes.ToStringLen(2) + ":" + remainder.ToStringLen(2);
 }

 static void Receive(GRZRotationSnapshot data)
 {
  Data = data;
  if (!data.Voting)
   PendingMenu = false;
  ReceivedAt = g_Game.GetTime();
  if (data.Enabled && data.Voting && data.RoundId != AutoPopupRound)
  {
   AutoPopupRound = data.RoundId;
   PendingMenu = true;
  }
  if (ManualRequest)
  {
   ManualRequest = false;
   if (!data.Enabled)
    ShowNotice(data.Status, 0);
   else if (data.Voting)
    PendingMenu = true;
   else
    ShowNotice("Voting is closed. It opens near the end of the round.", 0);
  }
  Updated.Invoke();
 }

 static void RequestSkip()
 {
  PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
  if (!player) return;
  ScriptRPC rpc = new ScriptRPC;
  rpc.Send(player, GRZRotationRPC.SKIP, true, null);
 }

 static void RequestMenu()
 {
  PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
  if (!player)
   return;
  ManualRequest = true;
  ScriptRPC rpc = new ScriptRPC;
  rpc.Send(player, GRZRotationRPC.REQUEST, true, null);
 }

 static void ShowNotice(string message, int sound)
 {
  if (message == "")
  {
   if (Notification)
    Notification.Show(false);
   NoticeUntil = 0;
   return;
  }
  if (!Notification)
   Notification = g_Game.GetWorkspace().CreateWidgets("GRZ_MapRotation/gui/layouts/notice.layout");
  if (Notification)
  {
   TextWidget label = TextWidget.Cast(Notification.FindAnyWidget("Message"));
   if (label)
    label.SetText(message);
   Notification.Show(true);
   NoticeUntil = g_Game.GetTime() + 6000;
  }
  if (sound <= 0)
   return;
  string name = "GRZ_Rotation_Vote_SoundSet";
  if (sound == 2)
   name = "GRZ_Rotation_Warning_SoundSet";
  if (sound == 3)
   name = "GRZ_Rotation_Change_SoundSet";
  if (Chime)
   SEffectManager.DestroyEffect(Chime);
  Chime = SEffectManager.CreateSound(name, g_Game.GetCurrentCameraPosition(), 0, 0, false, false);
  if (Chime)
  {
   Chime.SetSoundWaveKind(WaveKind.WAVEUI);
   Chime.SetSoundAutodestroy(true);
   Chime.SoundPlay();
  }
 }

 static void Update()
 {
  if (GRZDuelClientBus.Data && GRZDuelClientBus.Data.InMatch) return;
  if (Notification && g_Game.GetTime() >= NoticeUntil)
   Notification.Show(false);
  if (!PendingMenu || !Data || !Data.Enabled || !Data.Voting)
   return;
  if (!g_Game.IsAppActive() || g_Game.GetUIManager().GetMenu())
   return;
  if (g_Game.GetUIManager().IsDialogVisible())
   return;
  PendingMenu = false;
  g_Game.GetUIManager().EnterScriptedMenu(GRZRotationRPC.MENU, null);
 }

 static void Dispose()
 {
  if (Notification)
   Notification.Unlink();
  Notification = null;
  if (Chime)
   SEffectManager.DestroyEffect(Chime);
  Chime = null;
  Data = null;
  PendingMenu = false;
  ManualRequest = false;
  AutoPopupRound = -1;
 }
};

class GRZRotationVoteMenu : UIScriptedMenu
{
 protected float m_Age;

 override Widget Init()
 {
  layoutRoot = g_Game.GetWorkspace().CreateWidgets("GRZ_MapRotation/gui/layouts/vote.layout");
  ImageWidget logo = ImageWidget.Cast(layoutRoot.FindAnyWidget("Logo"));
  logo.LoadImageFile(0, "GRZ_MapRotation/data/logo.paa");
  logo.SetImage(0);
  logo.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND);
  GRZRotationClient.Updated.Insert(UpdateCards);
  UpdateCards();
  return layoutRoot;
 }

 override void OnHide()
 {
  GRZRotationClient.Updated.Remove(UpdateCards);
  super.OnHide();
 }

 void UpdateCards()
 {
  GRZRotationSnapshot data = GRZRotationClient.Data;
  if (!data || !data.Enabled || !data.Voting)
  {
   Close();
   return;
  }
  for (int i = 0; i < 6; i++)
  {
   string id = i.ToString();
   Widget card = layoutRoot.FindAnyWidget("Card" + id);
   bool present = i < data.Choices.Count();
   card.Show(present);
   if (!present)
    continue;
   if (data.Choices.Count() <= 3)
   {
    card.SetPos(0.035 + i * 0.32, 0.29);
    card.SetSize(0.29, 0.54);
   }
   GRZRotationChoice choice = data.Choices[i];
   TextWidget label = TextWidget.Cast(layoutRoot.FindAnyWidget("Name" + id));
   label.SetText(choice.Name);
   ImageWidget blueprint = ImageWidget.Cast(layoutRoot.FindAnyWidget("Blueprint" + id));
   blueprint.LoadImageFile(0, choice.Blueprint);
   blueprint.SetImage(0);
   string votes = choice.Votes.ToString() + " votes";
   if (choice.Id == data.ActiveId)
    votes = "CURRENT ARENA";
   else if (choice.Id == data.OwnVote)
    votes = "YOUR VOTE  |  " + votes;
   TextWidget tally = TextWidget.Cast(layoutRoot.FindAnyWidget("Votes" + id));
   tally.SetText(votes);
   Widget button = layoutRoot.FindAnyWidget("Vote" + id);
   button.Enable(choice.Id != data.ActiveId);
   Widget line = layoutRoot.FindAnyWidget("Selected" + id);
   line.Show(choice.Id == data.OwnVote);
  }
 }

 override bool OnClick(Widget w, int x, int y, int button)
 {
  if (button != MouseState.LEFT)
   return super.OnClick(w, x, y, button);
  if (w.GetName() == "Close")
  {
   Close();
   return true;
  }
  GRZRotationSnapshot data = GRZRotationClient.Data;
  if (!data || !data.Voting)
   return true;
  for (int i = 0; i < data.Choices.Count(); i++)
  {
   if (w.GetName() != "Vote" + i.ToString())
    continue;
   if (data.Choices[i].Id == data.ActiveId)
    return true;
   PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
   if (player)
   {
    ScriptRPC rpc = new ScriptRPC;
    rpc.Write(data.RoundId);
    rpc.Write(data.Choices[i].Id);
    rpc.Send(player, GRZRotationRPC.VOTE, true, null);
   }
   return true;
  }
  return super.OnClick(w, x, y, button);
 }

 override void Update(float timeslice)
 {
  super.Update(timeslice);
  m_Age += timeslice;
  GRZRotationSnapshot data = GRZRotationClient.Data;
  if (!data || !data.Voting || m_Age >= data.PopupSeconds)
  {
   Close();
   return;
  }
  int voteRemaining = Math.Max(0, GRZRotationClient.Remaining() - 30);
  TextWidget timer = TextWidget.Cast(layoutRoot.FindAnyWidget("Countdown"));
  int popupLeft = Math.Max(0, data.PopupSeconds - m_Age);
  timer.SetText("Voting closes in " + GRZRotationClient.Clock(voteRemaining) + "  |  Menu closes in " + popupLeft.ToString() + "s");
 }

 override bool OnKeyDown(Widget w, int x, int y, int key)
 {
  if (key == KeyCode.KC_ESCAPE)
  {
   Close();
   return true;
  }
  return super.OnKeyDown(w, x, y, key);
 }
};

modded class MissionBase
{
 override UIScriptedMenu CreateScriptedMenu(int id)
 {
  if (id == GRZRotationRPC.MENU)
  {
   GRZRotationVoteMenu menu = new GRZRotationVoteMenu;
   menu.SetID(id);
   return menu;
  }
  return super.CreateScriptedMenu(id);
 }
};

modded class MissionGameplay
{
 override void OnInit()
 {
  super.OnInit();
  GRZRotationBus.Snapshot.Insert(GRZRotationClient.Receive);
  GRZRotationBus.Notice.Insert(GRZRotationClient.ShowNotice);
 }

 override void OnUpdate(float timeslice)
 {
  super.OnUpdate(timeslice);
  GRZRotationClient.Update();
  if (!g_Game.GetUIManager().GetMenu())
  {
   UAInput input = GetUApi().GetInputByName("UAGRZMapVote");
   if (input && input.LocalPress())
    GRZRotationClient.RequestMenu();
  }
 }

 override void OnMissionFinish()
 {
  GRZRotationBus.Snapshot.Remove(GRZRotationClient.Receive);
  GRZRotationBus.Notice.Remove(GRZRotationClient.ShowNotice);
  GRZRotationClient.Dispose();
  g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(GRZRotationClient.RequestMenu);
  super.OnMissionFinish();
 }
};

modded class ChatInputMenu
{
 override bool OnChange(Widget w, int x, int y, bool finished)
 {
  if (finished)
  {
   EditBoxWidget edit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("InputEditBoxWidget"));
   if (edit)
   {
    string command = edit.GetText().Trim();
    command.ToLower();
    if (command == "!skip")
    {
     Close();
     GRZRotationClient.RequestSkip();
     return true;
    }
    if (command == "!vote")
    {
     Close();
     g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(GRZRotationClient.RequestMenu, 150, false);
     return true;
    }
   }
  }
  return super.OnChange(w, x, y, finished);
 }
};
