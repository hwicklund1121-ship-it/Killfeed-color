class GRZLoadoutMenu : UIScriptedMenu
{
 protected ItemPreviewWidget m_Preview;
 protected EntityAI m_PreviewGun;
 protected ref array<int> m_Filtered = new array<int>;
 protected int m_Category;
 protected int m_GunIndex;
 protected int m_WrapIndex;
 protected bool m_ClothingMode;
 protected int m_ClothingSlot;
 protected int m_ClothingIndex;
 protected PlayerPreviewWidget m_CharacterPreview;
 protected PlayerBase m_PreviewCharacter;
 protected TextListboxWidget m_ClothingList;
 protected bool m_ChoosingCharacter;
 protected string m_CharacterType;
 protected string m_TryOnType;
 protected string m_TryOnCharacter;
 protected int m_TryOnSlot = -1;
 protected bool m_PopulatingClothing;
 protected int m_LastPreviewRow = -1;
 protected ref array<int> m_ClothingFiltered = new array<int>;
 protected ref array<string> m_ClothingChoices = new array<string>;
 protected bool m_Saving;
 protected float m_SaveElapsed;

 override Widget Init()
 {
  layoutRoot = g_Game.GetWorkspace().CreateWidgets("GRZ_Loadout/gui/layouts/loadout.layout");
  m_CharacterPreview = PlayerPreviewWidget.Cast(layoutRoot.FindAnyWidget("CharacterPreview"));
  m_ClothingList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("ClothingList"));
  if (WidgetEventHandler.GetInstance())
  {
   WidgetEventHandler.GetInstance().RegisterOnMouseWheel(m_ClothingList, this, "GRZListWheel");
   WidgetEventHandler.GetInstance().RegisterOnDoubleClick(m_ClothingList, this, "OnDoubleClick");
  }
  m_Preview = ItemPreviewWidget.Cast(layoutRoot.FindAnyWidget("GunPreview"));
  ImageWidget logo = ImageWidget.Cast(layoutRoot.FindAnyWidget("Logo"));
  logo.LoadImageFile(0, "GRZ_Loadout/data/logo.paa");
  logo.SetImage(0);
  logo.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND);
  for (int clothingSlot = 0; clothingSlot < GRZLoadoutClothingSlots.Names.Count(); clothingSlot++)
   m_ClothingChoices.Insert("");
  GRZLoadoutClientData.Changed.Insert(ReceiveCatalogue);
  GRZLoadoutClientData.Result.Insert(ReceiveResult);
  Text("Status", "Loading available guns from server...");
  PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
  if (player)
  {
   m_CharacterType = player.GetType();
   ScriptRPC rpc = new ScriptRPC;
   rpc.Send(player, GRZLoadoutRPC.REQUEST, true, null);
  }
  if (GRZLoadoutClientData.Catalogue)
   ReceiveCatalogue();
  return layoutRoot;
 }

 override void OnHide()
 {
  if (m_ClothingList && WidgetEventHandler.GetInstance())
   WidgetEventHandler.GetInstance().UnregisterWidget(m_ClothingList);
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
  GRZLoadoutCatalogue received = GRZLoadoutClientData.Catalogue;
  if (received && received.SavedOutfit && received.SavedOutfit.Choices.Count() == m_ClothingChoices.Count())
  {
   m_CharacterType = received.SavedOutfit.Character;
   for (int savedSlot = 0; savedSlot < m_ClothingChoices.Count(); savedSlot++)
    m_ClothingChoices[savedSlot] = received.SavedOutfit.Choices[savedSlot];
  }
  if (m_ClothingMode)
   FilterClothing();
  else
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
  if (m_ClothingMode)
  {
   RefreshClothing();
   return;
  }
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
  if (m_CharacterPreview)
   m_CharacterPreview.SetPlayer(null);
  if (m_PreviewCharacter && g_Game)
   g_Game.ObjectDelete(m_PreviewCharacter);
  m_PreviewCharacter = null;
  if (m_Preview)
   m_Preview.SetItem(null);
  if (m_PreviewGun && g_Game)
   g_Game.ObjectDelete(m_PreviewGun);
  m_PreviewGun = null;
 }

 void RebuildPreview()
 {
  DisposePreview();
  if (m_ClothingMode)
  {
   PreviewClothing();
   return;
  }
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

 protected void SelectPage(bool clothing)
 {
  ClearTryOn();
  m_ClothingMode = clothing;
  string gunWidgets[12] = {"Calibre20", "Calibre50", "Calibre408", "GunPreview", "GunPrevious", "GunNext", "GunName", "GunCount", "WrapHeading", "WrapPrevious", "WrapNext", "WrapName"};
  for (int i = 0; i < 12; i++)
   layoutRoot.FindAnyWidget(gunWidgets[i]).Show(!clothing);
  string lines[3] = {"Calibre20Line", "Calibre50Line", "Calibre408Line"};
  foreach (string line : lines)
   layoutRoot.FindAnyWidget(line).Show(false);
  layoutRoot.FindAnyWidget("PreviewBackground").Show(!clothing);
  layoutRoot.FindAnyWidget("ClothingPanel").Show(clothing);
  int light = ARGB(255, 245, 245, 245);
  int dark = ARGB(255, 22, 22, 24);
  if (clothing)
  {
   layoutRoot.FindAnyWidget("LoadoutTabFill").SetColor(dark);
   layoutRoot.FindAnyWidget("LoadoutTabLabel").SetColor(light);
   layoutRoot.FindAnyWidget("ClothingTabFill").SetColor(light);
   layoutRoot.FindAnyWidget("ClothingTabLabel").SetColor(dark);
   Text("SaveLabel", "SAVE & APPLY");
   FilterClothing();
  }
  else
  {
   layoutRoot.FindAnyWidget("LoadoutTabFill").SetColor(light);
   layoutRoot.FindAnyWidget("LoadoutTabLabel").SetColor(dark);
   layoutRoot.FindAnyWidget("ClothingTabFill").SetColor(dark);
   layoutRoot.FindAnyWidget("ClothingTabLabel").SetColor(light);
   Text("SaveLabel", "SAVE LOADOUT");
   RefreshChoices();
  }
 }

 protected void FilterClothing()
 {
  ClearTryOn();
  m_PopulatingClothing = true;
  m_LastPreviewRow = -1;
  m_ClothingFiltered.Clear();
  m_ClothingList.ClearItems();
  GRZLoadoutCatalogue catalogue = GRZLoadoutClientData.Catalogue;
  if (catalogue)
  {
   if (m_ChoosingCharacter)
   {
    foreach (string character : catalogue.Characters)
    {
     string label = character;
     label.Replace("SurvivorM_", "");
     label.Replace("SurvivorF_", "");
     m_ClothingList.AddItem(label, null, 0);
    }
   }
   else
   {
    string slot = GRZLoadoutClothingSlots.Names[m_ClothingSlot];
    for (int i = 0; i < catalogue.Clothing.Count(); i++)
    {
     GRZLoadoutClothing item = catalogue.Clothing[i];
     if (item && item.Slot == slot)
     {
      m_ClothingFiltered.Insert(i);
      m_ClothingList.AddItem(GRZClothingLabels.Label(item.Type, item.Name), null, 0);
     }
    }
   }
  }
  m_PopulatingClothing = false;
  RefreshClothing();
 }

 protected void RefreshClothing()
 {
  layoutRoot.FindAnyWidget("Save").Enable(!m_Saving && GRZLoadoutClientData.Catalogue);
  string title = "Scroll to preview / double-click to keep";
  if (m_ChoosingCharacter)
   title = "Scroll to preview / double-click to keep";
  Text("ClothingListTitle", title);
  layoutRoot.FindAnyWidget("RemoveClothing").Enable(!m_ChoosingCharacter);
  string names[8] = {"SlotHat", "SlotMask", "SlotShirt", "SlotBelt", "SlotPants", "SlotGlove", "SlotVest", "SlotShoes"};
  int indices[8] = {0, 1, 3, 9, 6, 5, 4, 7};
  for (int i = 0; i < 8; i++)
  {
   int color = ARGB(255, 25, 25, 28);
   if (!m_ChoosingCharacter && indices[i] == m_ClothingSlot)
    color = ARGB(255, 85, 85, 90);
   layoutRoot.FindAnyWidget(names[i] + "Fill").SetColor(color);
  }
  g_Game.GetCallQueue(CALL_CATEGORY_GUI).Remove(RebuildPreview);
  g_Game.GetCallQueue(CALL_CATEGORY_GUI).CallLater(RebuildPreview, 1, false);
 }

 protected void PreviewClothing()
 {
  PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
  string previewCharacter = m_CharacterType;
  if (m_TryOnCharacter != "")
   previewCharacter = m_TryOnCharacter;
  if (!player || previewCharacter == "")
   return;
  int flags = ECE_LOCAL | ECE_NOLIFETIME | ECE_KEEPHEIGHT | ECE_NOSURFACEALIGN | ECE_NOPERSISTENCY_WORLD | ECE_NOPERSISTENCY_CHAR;
  m_PreviewCharacter = PlayerBase.Cast(g_Game.CreateObjectEx(previewCharacter, "0 -1000 0", flags));
  if (!m_PreviewCharacter)
  {
   Text("Status", "Character preview could not be created.");
   return;
  }
  // Only preview clones are dressed here; the real inventory changes on Save.
  for (int i = 0; i < m_ClothingChoices.Count(); i++)
  {
   string type = m_ClothingChoices[i];
   if (i == m_TryOnSlot)
    type = m_TryOnType;
   if (type == "__REMOVE__")
    continue;
   if (type == "")
   {
    EntityAI current = player.FindAttachmentBySlotName(GRZLoadoutClothingSlots.Names[i]);
    if (current)
     type = current.GetType();
   }
   if (type != "")
   {
    int slot = InventorySlots.GetSlotIdFromString(GRZLoadoutClothingSlots.Names[i]);
    if (!m_PreviewCharacter.GetInventory().CreateAttachmentEx(type, slot))
     Text("Status", "Some selected clothing cannot be worn together. Choose a different combination.");
   }
  }
  m_CharacterPreview.SetPlayer(m_PreviewCharacter);
  m_CharacterPreview.SetModelPosition("0 0 0.605");
  m_CharacterPreview.SetModelOrientation("0 0 0");
  m_CharacterPreview.Refresh();
 }

 protected bool ClothingClick(string name)
 {
  string names[8] = {"SlotHat", "SlotMask", "SlotShirt", "SlotBelt", "SlotPants", "SlotGlove", "SlotVest", "SlotShoes"};
  int indices[8] = {0, 1, 3, 9, 6, 5, 4, 7};
  for (int i = 0; i < 8; i++)
  {
   if (name == names[i])
   {
    m_ClothingSlot = indices[i];
    m_ChoosingCharacter = false;
    FilterClothing();
    return true;
   }
  }
  if (name == "SelectCharacter")
  {
   m_ChoosingCharacter = true;
   FilterClothing();
   return true;
  }
  if (name == "RemoveClothing")
  {
   ClearTryOn();
   m_ClothingChoices[m_ClothingSlot] = "__REMOVE__";
   Text("Status", "");
   RefreshClothing();
   return true;
  }
  if (name == "Save")
  {
   ClearTryOn();
   RefreshClothing();
   PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
   if (player && player.IsAlive())
   {
    ScriptRPC rpc = new ScriptRPC;
    rpc.Write(m_ClothingChoices);
    rpc.Write(m_CharacterType);
    rpc.Send(player, GRZLoadoutRPC.SAVE_CLOTHING, true, null);
    m_Saving = true;
    m_SaveElapsed = 0;
    layoutRoot.FindAnyWidget("Save").Enable(false);
    Text("Status", "Saving and applying outfit...");
   }
   return true;
  }
  return false;
 }

 protected void ClearTryOn()
 {
  m_TryOnType = "";
  m_TryOnCharacter = "";
  m_TryOnSlot = -1;
 }

 protected void PreviewClothingRow(int row)
 {
  GRZLoadoutCatalogue catalogue = GRZLoadoutClientData.Catalogue;
  if (!catalogue || row < 0)
   return;
  m_LastPreviewRow = row;
  ClearTryOn();
  if (m_ChoosingCharacter)
  {
   if (row < catalogue.Characters.Count())
    m_TryOnCharacter = catalogue.Characters[row];
  }
  else if (row < m_ClothingFiltered.Count())
  {
   m_TryOnSlot = m_ClothingSlot;
   m_TryOnType = catalogue.Clothing[m_ClothingFiltered[row]].Type;
  }
  RefreshClothing();
 }

 bool GRZListWheel(Widget w, int x, int y, int wheel)
 {
  if (!m_ClothingMode || m_Saving || m_PopulatingClothing || wheel == 0)
   return false;
  int count = m_ClothingList.GetNumItems();
  if (count == 0)
   return true;
  int row = m_ClothingList.GetSelectedRow();
  if (row < 0)
   row = 0;
  else if (wheel < 0)
   row = Math.Min(row + 1, count - 1);
  else
   row = Math.Max(row - 1, 0);
  m_ClothingList.SelectRow(row);
  m_ClothingList.EnsureVisible(row);
  PreviewClothingRow(row);
  return true;
 }

 override bool OnMouseWheel(Widget w, int x, int y, int wheel)
 {
  Widget target = w;
  while (target && target != m_ClothingList)
   target = target.GetParent();
  if (target == m_ClothingList)
   return GRZListWheel(w, x, y, wheel);
  return super.OnMouseWheel(w, x, y, wheel);
 }

 override bool OnDoubleClick(Widget w, int x, int y, int button)
 {
  if (m_ClothingMode && !m_Saving && w == m_ClothingList && button == MouseState.LEFT)
  {
   GRZLoadoutCatalogue catalogue = GRZLoadoutClientData.Catalogue;
   int row = m_ClothingList.GetSelectedRow();
   if (!catalogue || row < 0)
    return true;
   if (m_ChoosingCharacter)
   {
    if (row < catalogue.Characters.Count())
     m_CharacterType = catalogue.Characters[row];
   }
   else if (row < m_ClothingFiltered.Count())
    m_ClothingChoices[m_ClothingSlot] = catalogue.Clothing[m_ClothingFiltered[row]].Type;
   ClearTryOn();
   Text("Status", "");
   RefreshClothing();
   return true;
  }
  return super.OnDoubleClick(w, x, y, button);
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
  if (name == "DuelsTab")
  {
   Close();
   g_Game.GetUIManager().EnterScriptedMenu(GRZDuelRPC.MENU,null);
   return true;
  }
  if (name == "ClothingTab" || name == "LoadoutTab")
  {
   SelectPage(name == "ClothingTab");
   return true;
  }
  if (m_ClothingMode && ClothingClick(name))
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
  // Also follow native highlight changes if the control handles an input itself.
  if (m_ClothingMode && !m_Saving && !m_PopulatingClothing && m_ClothingList)
  {
   int highlighted = m_ClothingList.GetSelectedRow();
   if (highlighted >= 0 && highlighted != m_LastPreviewRow)
    PreviewClothingRow(highlighted);
  }
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
