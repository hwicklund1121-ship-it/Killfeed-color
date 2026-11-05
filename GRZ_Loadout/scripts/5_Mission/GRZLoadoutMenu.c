class GRZLoadoutMenu : UIScriptedMenu
{
 protected ItemPreviewWidget m_Preview;
 protected EntityAI m_PreviewGun;
 protected ref array<int> m_Filtered = new array<int>;
 protected int m_Category;
 protected int m_GunIndex;
 protected int m_WrapIndex;
 protected bool m_Saving;
 protected float m_SaveElapsed;

 override Widget Init()
 {
  layoutRoot = g_Game.GetWorkspace().CreateWidgets("GRZ_Loadout/gui/layouts/loadout.layout");
  m_Preview = ItemPreviewWidget.Cast(layoutRoot.FindAnyWidget("GunPreview"));
  ImageWidget logo = ImageWidget.Cast(layoutRoot.FindAnyWidget("Logo"));
  logo.LoadImageFile(0, "GRZ_Loadout/data/logo.paa");
  logo.SetImage(0);
  logo.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND);
  layoutRoot.FindAnyWidget("ClothingTab").Enable(false);
  GRZLoadoutClientData.Changed.Insert(ReceiveCatalogue);
  GRZLoadoutClientData.Result.Insert(ReceiveResult);
  Text("Status", "Loading available guns from server...");
  PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
  if (player)
  {
   ScriptRPC rpc = new ScriptRPC;
   rpc.Send(player, GRZLoadoutRPC.REQUEST, true, null);
  }
  if (GRZLoadoutClientData.Catalogue)
   ReceiveCatalogue();
  return layoutRoot;
 }

 override void OnHide()
 {
  GRZLoadoutClientData.Changed.Remove(ReceiveCatalogue);
  GRZLoadoutClientData.Result.Remove(ReceiveResult);
  g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(RebuildPreview);
  DisposePreview();
  super.OnHide();
 }

 void ~GRZLoadoutMenu()
 {
  GRZLoadoutClientData.Changed.Remove(ReceiveCatalogue);
  GRZLoadoutClientData.Result.Remove(ReceiveResult);
  if (g_Game)
   g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(RebuildPreview);
  DisposePreview();
 }

 protected void Text(string name, string value)
 {
  TextWidget widget = TextWidget.Cast(layoutRoot.FindAnyWidget(name));
  if (widget)
   widget.SetText(value);
 }

 void ReceiveCatalogue()
 {
  m_GunIndex = 0;
  FilterGuns();
  Text("Status", "");
 }

 void ReceiveResult(string message)
 {
  m_Saving = false;
  layoutRoot.FindAnyWidget("Save").Enable(true);
  Text("Status", message);
 }

 protected void FilterGuns()
 {
  m_Filtered.Clear();
  GRZLoadoutCatalogue catalogue = GRZLoadoutClientData.Catalogue;
  if (catalogue && catalogue.Guns)
  {
   for (int i = 0; i < catalogue.Guns.Count(); i++)
   {
    if (catalogue.Guns[i] && catalogue.Guns[i].Category == m_Category)
     m_Filtered.Insert(i);
   }
  }
  m_GunIndex = 0;
  RefreshChoices();
 }

 protected GRZLoadoutGun CurrentGun()
 {
  GRZLoadoutCatalogue catalogue = GRZLoadoutClientData.Catalogue;
  if (!catalogue || m_Filtered.Count() == 0)
   return null;
  return catalogue.Guns[m_Filtered[m_GunIndex]];
 }

 protected string CurrentWrap()
 {
  GRZLoadoutCatalogue catalogue = GRZLoadoutClientData.Catalogue;
  if (!catalogue || !catalogue.Wraps || catalogue.Wraps.Count() == 0)
   return "";
  m_WrapIndex = m_WrapIndex % catalogue.Wraps.Count();
  return catalogue.Wraps[m_WrapIndex];
 }

 protected void RefreshChoices()
 {
  if (!m_Saving)
   Text("Status", "");
  GRZLoadoutGun gun = CurrentGun();
  string name = "No guns installed in this section";
  if (gun)
   name = gun.Name;
  Text("GunName", name);
  string position = "0 / 0";
  if (gun)
   position = string.Format("%1 / %2", m_GunIndex + 1, m_Filtered.Count());
  Text("GunCount", position);
  string wrap = CurrentWrap();
  string wrapName = "No wrap";
  if (wrap != "")
  {
   g_Game.ConfigGetText("CfgVehicles " + wrap + " displayName", wrapName);
   if (wrapName == "")
    wrapName = wrap;
  }
  Text("WrapName", wrapName);
  string tabs[3] = {"Calibre20", "Calibre50", "Calibre408"};
  for (int i = 0; i < 3; i++)
  {
   int color = ARGB(255, 145, 145, 145);
   if (i == m_Category)
    color = ARGB(255, 255, 255, 255);
   layoutRoot.FindAnyWidget(tabs[i]).SetColor(color);
   layoutRoot.FindAnyWidget(tabs[i] + "Line").Show(i == m_Category);
  }
  layoutRoot.FindAnyWidget("Save").Enable(gun && !m_Saving);
  g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(RebuildPreview);
  g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(RebuildPreview, 1, false);
 }

 protected void DisposePreview()
 {
  if (m_Preview)
   m_Preview.SetItem(null);
  if (m_PreviewGun && g_Game)
   g_Game.ObjectDelete(m_PreviewGun);
  m_PreviewGun = null;
 }

 void RebuildPreview()
 {
  DisposePreview();
  GRZLoadoutGun gun = CurrentGun();
  if (!gun || !m_Preview)
   return;
  int flags = ECE_LOCAL | ECE_NOLIFETIME | ECE_KEEPHEIGHT | ECE_NOSURFACEALIGN | ECE_NOPERSISTENCY_WORLD | ECE_NOPERSISTENCY_CHAR;
  m_PreviewGun = EntityAI.Cast(g_Game.CreateObjectEx(gun.Type, "0 -1000 0", flags));
  if (!m_PreviewGun)
  {
   Text("Status", "This gun's preview model could not be loaded.");
   return;
  }
  if (!GRZLoadoutAttachments.EnsurePermanentSFGSuppressor(m_PreviewGun, false))
   Text("Status", "SFG suppressor preview could not be attached. Check the SFG weapon pack.");
  if (gun.ScopeType != "")
   m_PreviewGun.GetInventory().CreateAttachment(gun.ScopeType);
  string wrap = CurrentWrap();
  if (wrap != "" && !m_PreviewGun.GetInventory().CreateAttachment(wrap))
   Text("Status", "This gun does not accept the selected wrap. Choose another wrap.");
  m_Preview.SetItem(m_PreviewGun);
  m_Preview.SetView(m_PreviewGun.GetViewIndex());
  m_Preview.SetModelPosition("0 0 0.8");
  m_Preview.SetModelOrientation("0 0 0");
 }

 override bool OnClick(Widget w, int x, int y, int button)
 {
  if (button != MouseState.LEFT)
   return super.OnClick(w, x, y, button);
  string name = w.GetName();
  if (name == "Close")
  {
   Close();
   return true;
  }
  if (m_Saving)
   return true;
  if (name == "Calibre20" || name == "Calibre50" || name == "Calibre408")
  {
   m_Category = 0;
   if (name == "Calibre50")
    m_Category = 1;
   if (name == "Calibre408")
    m_Category = 2;
   FilterGuns();
   return true;
  }
  if (name == "GunPrevious" || name == "GunNext")
  {
   if (m_Filtered.Count() > 0)
   {
    int delta = 1;
    if (name == "GunPrevious")
     delta = -1;
    m_GunIndex = (m_GunIndex + delta + m_Filtered.Count()) % m_Filtered.Count();
    RefreshChoices();
   }
   return true;
  }
  if (name == "WrapPrevious" || name == "WrapNext")
  {
   GRZLoadoutCatalogue catalogue = GRZLoadoutClientData.Catalogue;
   if (catalogue && catalogue.Wraps.Count() > 0)
   {
    int wrapDelta = 1;
    if (name == "WrapPrevious")
     wrapDelta = -1;
    m_WrapIndex = (m_WrapIndex + wrapDelta + catalogue.Wraps.Count()) % catalogue.Wraps.Count();
    RefreshChoices();
   }
   return true;
  }
  if (name == "Save")
  {
   GRZLoadoutGun gun = CurrentGun();
   PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
   if (gun && player && player.IsAlive())
   {
    ScriptRPC rpc = new ScriptRPC;
    rpc.Write(gun.Type);
    rpc.Write(CurrentWrap());
    rpc.Send(player, GRZLoadoutRPC.SAVE, true, null);
    m_Saving = true;
    m_SaveElapsed = 0;
    w.Enable(false);
    Text("Status", "Equipping loadout...");
   }
   return true;
  }
  return super.OnClick(w, x, y, button);
 }

 override void Update(float timeslice)
 {
  super.Update(timeslice);
  PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
  if (!player || !player.IsAlive() || player.IsUnconscious())
  {
   Close();
   return;
  }
  if (m_Saving)
  {
   m_SaveElapsed += timeslice;
   if (m_SaveElapsed > 15)
    ReceiveResult("Server response timed out. Check inventory before retrying.");
  }
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
  if (id == GRZLoadoutRPC.MENU)
  {
   GRZLoadoutMenu menu = new GRZLoadoutMenu;
   menu.SetID(id);
   return menu;
  }
  return super.CreateScriptedMenu(id);
 }
};

modded class MissionGameplay
{
 override void OnUpdate(float timeslice)
 {
  super.OnUpdate(timeslice);
  if (!g_Game.IsAppActive() || g_Game.GetUIManager().GetMenu() || g_Game.GetUIManager().IsDialogVisible())
   return;
  PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
  if (!player || !player.IsAlive() || player.IsUnconscious())
   return;
  UAInput input = GetUApi().GetInputByName("UAGRZLoadout");
  if (input && input.LocalPress())
   g_Game.GetUIManager().EnterScriptedMenu(GRZLoadoutRPC.MENU, null);
 }
};
