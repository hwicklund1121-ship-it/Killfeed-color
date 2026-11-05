class GRZLoadoutServer
{
 static ref GRZLoadoutSettings Settings;
 static ref GRZLoadoutCatalogue Catalogue;

 static int Calibre(string ammo)
 {
  string lower = ammo;
  lower.ToLower();
  lower.Replace("_", "");
  lower.Replace("-", "");
  lower.Replace(" ", "");
  if (lower.Contains("vulcan") || lower.Contains("20mm"))
   return 0;
  if (lower.Contains("20x102") || lower.Contains("20x110") || lower.Contains("20x138"))
   return 0;
  if (lower.Contains("50cal") || lower.Contains("50bmg"))
   return 1;
  if (lower.Contains("127x99") || lower.Contains("127x108"))
   return 1;
  if (lower.Contains("12.7x99") || lower.Contains("12.7x108"))
   return 1;
  if (lower.Contains("408"))
   return 2;
  return -1;
 }

 static int AmmoCategory(string ammo)
 {
  string bullet;
  g_Game.ConfigGetText("CfgMagazines " + ammo + " ammo", bullet);
  int category = Calibre(bullet);
  if (category < 0)
   category = Calibre(ammo);
  if (category < 0)
  {
   string display;
   g_Game.ConfigGetText("CfgMagazines " + ammo + " displayName", display);
   category = Calibre(display);
  }
  return category;
 }

 static bool ExcludedGun(string type)
 {
  string display;
  string model;
  g_Game.ConfigGetText("CfgWeapons " + type + " displayName", display);
  g_Game.ConfigGetText("CfgWeapons " + type + " model", model);
  string identity = type + " " + display + " " + model;
  identity.ToLower();
  identity.Replace("_", "");
  identity.Replace("-", "");
  identity.Replace(" ", "");
  if (identity.Contains("as50"))
   return true;
  if (identity.Contains("barrett") || identity.Contains("barret"))
   return true;
  if (identity.Contains("m82") || identity.Contains("m107"))
   return true;
  if (identity.Contains("m95") || identity.Contains("m99"))
   return true;
  return false;
 }

 static bool ScopeFits(string weapon, string scope)
 {
  if (g_Game.ConfigGetInt("CfgVehicles " + scope + " scope") != 2)
   return false;
  if (!g_Game.IsKindOf(scope, "ItemOptics"))
   return false;
  array<string> mounts = new array<string>;
  array<string> slots = new array<string>;
  g_Game.ConfigGetTextArray("CfgWeapons " + weapon + " attachments", mounts);
  g_Game.ConfigGetTextArray("CfgVehicles " + scope + " inventorySlot", slots);
  foreach (string slot : slots)
  {
   if (mounts.Find(slot) >= 0)
    return true;
  }
  return false;
 }

 static string FindScope(string weapon)
 {
  array<string> preferred = {"HuntingOptic", "ACOGOptic", "PSO1Optic", "KashtanOptic", "PUScopeOptic"};
  foreach (string optic : preferred)
  {
   if (ScopeFits(weapon, optic))
    return optic;
  }
  int count = g_Game.ConfigGetChildrenCount("CfgVehicles");
  for (int i = 0; i < count; i++)
  {
   string type;
   g_Game.ConfigGetChildName("CfgVehicles", i, type);
   if (ScopeFits(weapon, type))
    return type;
  }
  return "";
 }

 static bool ValidGun(GRZLoadoutGun entry)
 {
  if (!entry || entry.Category < 0 || entry.Category > 2)
   return false;
  if (ExcludedGun(entry.Type))
   return false;
  if (!g_Game.IsKindOf(entry.Type, "BoltActionRifle_Base") && !g_Game.IsKindOf(entry.Type, "BoltActionRifle_InnerMagazine_Base"))
   return false;
  if (g_Game.ConfigGetInt("CfgWeapons " + entry.Type + " scope") != 2)
   return false;
  if (!g_Game.ConfigIsExisting("CfgMagazines " + entry.Ammo))
   return false;
  if (!g_Game.IsKindOf(entry.Ammo, "Ammunition_Base"))
   return false;
  if (entry.MagazineType != "" && !g_Game.ConfigIsExisting("CfgMagazines " + entry.MagazineType))
   return false;
  if (entry.MagazineType != "")
  {
   string pileBullet;
   string magazineBullet;
   g_Game.ConfigGetText("CfgMagazines " + entry.Ammo + " ammo", pileBullet);
   g_Game.ConfigGetText("CfgMagazines " + entry.MagazineType + " ammo", magazineBullet);
   if (pileBullet == "" || pileBullet != magazineBullet)
    return false;
  }
  entry.ScopeType = FindScope(entry.Type);
  if (entry.Name == "")
   g_Game.ConfigGetText("CfgWeapons " + entry.Type + " displayName", entry.Name);
  if (entry.Name == "")
   entry.Name = entry.Type;
  return true;
 }

 static GRZLoadoutGun Discover(string weapon)
 {
  array<string> chamber = new array<string>;
  array<string> magazines = new array<string>;
  g_Game.ConfigGetTextArray("CfgWeapons " + weapon + " chamberableFrom", chamber);
  g_Game.ConfigGetTextArray("CfgWeapons " + weapon + " magazines", magazines);
  GRZLoadoutGun entry = new GRZLoadoutGun;
  entry.Type = weapon;
  entry.Category = -1;
  foreach (string pile : chamber)
  {
   int category = AmmoCategory(pile);
   if (category >= 0)
   {
    entry.Ammo = pile;
    entry.Category = category;
    break;
   }
  }
  foreach (string mag : magazines)
  {
   int magCategory = AmmoCategory(mag);
   if (magCategory < 0)
    continue;
   if (entry.Category >= 0 && magCategory != entry.Category)
    continue;
   entry.MagazineType = mag;
   entry.Category = magCategory;
   if (entry.Ammo == "")
    entry.Ammo = mag;
   break;
  }
  if (entry.MagazineType != "" && entry.Ammo == entry.MagazineType)
  {
   string bullet;
   g_Game.ConfigGetText("CfgMagazines " + entry.MagazineType + " ammo", bullet);
   int ammoCount = g_Game.ConfigGetChildrenCount("CfgMagazines");
   for (int a = 0; a < ammoCount; a++)
   {
    string candidate;
    string candidateBullet;
    g_Game.ConfigGetChildName("CfgMagazines", a, candidate);
    if (!g_Game.IsKindOf(candidate, "Ammunition_Base"))
     continue;
    g_Game.ConfigGetText("CfgMagazines " + candidate + " ammo", candidateBullet);
    if (candidateBullet == bullet && g_Game.ConfigGetInt("CfgMagazines " + candidate + " scope") == 2)
    {
     entry.Ammo = candidate;
     break;
    }
   }
  }
  if (!ValidGun(entry))
   return null;
  return entry;
 }

 static void Init()
 {
  if (Catalogue)
   return;
  Settings = new GRZLoadoutSettings;
  string directory = "$profile:GRZ_Loadout";
  MakeDirectory(directory);
  string path = directory + "/config.json";
  if (FileExist(path))
  {
   string error;
   if (!JsonFileLoader<GRZLoadoutSettings>.LoadFile(path, Settings, error))
   {
    Print("[GRZ Loadout] Invalid config; loadout grants disabled: " + error);
    Settings = new GRZLoadoutSettings;
    Settings.Enabled = false;
   }
  }
  else
   JsonFileLoader<GRZLoadoutSettings>.JsonSaveFile(path, Settings);
  Catalogue = new GRZLoadoutCatalogue;
  Catalogue.Characters = g_Game.ListAvailableCharacters();
  Catalogue.AmmoStacks = Math.Clamp(Settings.AmmoStacks, 1, 30);
  if (!Settings.Enabled)
   return;
  array<string> seen = new array<string>;
  if (Settings.Guns)
  {
   foreach (GRZLoadoutGun custom : Settings.Guns)
   {
    if (ValidGun(custom) && seen.Find(custom.Type) < 0)
    {
     Catalogue.Guns.Insert(custom);
     seen.Insert(custom.Type);
    }
   }
  }
  int count = g_Game.ConfigGetChildrenCount("CfgWeapons");
  if (Settings.AutoDiscover)
  {
   for (int i = 0; i < count; i++)
   {
    string weapon;
    g_Game.ConfigGetChildName("CfgWeapons", i, weapon);
    if (seen.Find(weapon) >= 0 || g_Game.ConfigGetInt("CfgWeapons " + weapon + " scope") != 2)
     continue;
    GRZLoadoutGun found = Discover(weapon);
    if (found)
    {
     Catalogue.Guns.Insert(found);
     seen.Insert(weapon);
    }
   }
  }
  GRZLoadoutClothingServer.Discover(Catalogue, Settings.AllInstalledClothing);
  // Native wraps and installed GRZ/custom gun wraps, never full ghillie suits.
  Catalogue.Wraps.Insert("");
  count = g_Game.ConfigGetChildrenCount("CfgVehicles");
  for (int j = 0; j < count; j++)
  {
   string wrap;
   g_Game.ConfigGetChildName("CfgVehicles", j, wrap);
   if (g_Game.ConfigGetInt("CfgVehicles " + wrap + " scope") != 2)
    continue;
   if (g_Game.IsKindOf(wrap, "GhillieAtt_ColorBase"))
    Catalogue.Wraps.Insert(wrap);
  }
  Print(string.Format("[GRZ Loadout] Registered %1 guns and %2 wrap options.", Catalogue.Guns.Count(), Catalogue.Wraps.Count()));
 }

 static void Reply(PlayerBase player, string message)
 {
  ScriptRPC rpc = new ScriptRPC;
  rpc.Write(message);
  rpc.Send(player, GRZLoadoutRPC.RESULT, true, player.GetIdentity());
 }

 static bool MoveTo(EntityAI item, InventoryLocation destination)
 {
  InventoryLocation current = new InventoryLocation;
  if (!item || !item.GetInventory().GetCurrentInventoryLocation(current))
   return false;
  if (!GameInventory.LocationCanMoveEntity(current, destination))
   return false;
  return GameInventory.LocationSyncMoveEntity(current, destination);
 }

 static void DeleteStaged(array<EntityAI> staged)
 {
  foreach (EntityAI item : staged)
  {
   if (item)
    g_Game.ObjectDelete(item);
  }
 }

 static void Restore(PlayerBase player, array<EntityAI> oldItems, array<ref InventoryLocation> original)
 {
  for (int i = 0; i < oldItems.Count(); i++)
  {
   if (!MoveTo(oldItems[i], original[i]))
   {
    // Preserve the item even if an external inventory mod rejects the rollback.
    oldItems[i].SetPosition(player.GetPosition());
    Error("[GRZ Loadout] Could not restore an old item location; item returned beside owner.");
   }
  }
 }

 static bool Equip(PlayerBase player, string weaponType, string wrapType, bool remember = true)
 {
  Init();
  bool unavailable = !Settings.Enabled || !player.IsAlive() || player.IsUnconscious();
  if (player.IsRestrained() || player.IsRaised() || player.GetCommand_Vehicle())
   unavailable = true;
  if (unavailable)
  {
   Reply(player, "Loadout unavailable right now.");
   return false;
  }
  GRZLoadoutGun selected;
  foreach (GRZLoadoutGun entry : Catalogue.Guns)
  {
   if (entry.Type == weaponType)
   {
    selected = entry;
    break;
   }
  }
  if (!selected || Catalogue.Wraps.Find(wrapType) < 0)
  {
   Reply(player, "Selection is not in the server's loadout list.");
   return false;
  }
  EntityAI hands = player.GetHumanInventory().GetEntityInHands();
  if (hands && !Weapon_Base.Cast(hands) && !Magazine.Cast(hands))
  {
   Reply(player, "Put the item in your hands away first.");
   return false;
  }
  vector stagingPosition = player.GetPosition() + "0 -1000 0";
  array<EntityAI> staged = new array<EntityAI>;
  int flags = ECE_KEEPHEIGHT | ECE_NOSURFACEALIGN | ECE_NOLIFETIME;
  Weapon_Base gun = Weapon_Base.Cast(g_Game.CreateObjectEx(selected.Type, stagingPosition, flags));
  if (!gun)
  {
   Reply(player, "Gun creation failed. Your inventory was kept.");
   return false;
  }
  staged.Insert(gun);
  if (!GRZLoadoutAttachments.EnsurePermanentSFGSuppressor(gun, true))
  {
   DeleteStaged(staged);
   Reply(player, "SFG suppressor is missing or incompatible. Update the SFG weapon pack. Your inventory was kept.");
   return false;
  }
  if (selected.ScopeType != "")
  {
   if (!gun.GetInventory().CreateAttachment(selected.ScopeType))
   {
    DeleteStaged(staged);
    Reply(player, "Scope attachment failed. Your inventory was kept.");
    return false;
   }
  }
  if (wrapType != "")
  {
   EntityAI wrap = gun.GetInventory().CreateAttachment(wrapType);
   if (!wrap)
   {
    DeleteStaged(staged);
    Reply(player, "That gun cannot accept this wrap. Your inventory was kept.");
    return false;
   }
  }
  string loadType = selected.Ammo;
  if (selected.MagazineType != "")
   loadType = selected.MagazineType;
  gun.SpawnAmmo(loadType);
  if (gun.IsChamberEmpty(0))
  {
   DeleteStaged(staged);
   Reply(player, "Gun could not chamber its configured ammo. Your inventory was kept.");
   return false;
  }
  bool prepared = true;
  for (int n = 0; n < Catalogue.AmmoStacks; n++)
  {
   Magazine spare = Magazine.Cast(g_Game.CreateObjectEx(selected.Ammo, stagingPosition, flags));
   if (!spare)
   {
    prepared = false;
    break;
   }
   staged.Insert(spare);
   if (!spare.IsAmmoPile() || spare.GetAmmoMax() <= 0)
   {
    prepared = false;
    break;
   }
   spare.ServerSetAmmoCount(spare.GetAmmoMax());
  }
  if (!prepared)
  {
   DeleteStaged(staged);
   Reply(player, "Ammo creation failed. Your inventory was kept.");
   return false;
  }
  if (selected.MagazineType != "")
  {
   for (int spareIndex = 0; spareIndex < 6; spareIndex++)
   {
    Magazine spareMagazine = Magazine.Cast(g_Game.CreateObjectEx(selected.MagazineType, stagingPosition, flags));
    if (!spareMagazine)
    {
     DeleteStaged(staged);
     Reply(player, "Magazine creation failed. Your inventory was kept.");
     return false;
    }
    staged.Insert(spareMagazine);
    spareMagazine.ServerSetAmmoCount(spareMagazine.GetAmmoMax());
   }
  }
  // Select topmost weapon/ammo entities so attachments are not moved twice.
  array<EntityAI> inventory = new array<EntityAI>;
  player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, inventory);
  array<EntityAI> oldItems = new array<EntityAI>;
  array<ref InventoryLocation> original = new array<ref InventoryLocation>;
  foreach (EntityAI old : inventory)
  {
   if (!old || (!Weapon_Base.Cast(old) && !Magazine.Cast(old)))
    continue;
   EntityAI ancestor = old.GetHierarchyParent();
   bool nested = false;
   while (ancestor && ancestor != player)
   {
    if (Weapon_Base.Cast(ancestor) || Magazine.Cast(ancestor))
    {
     nested = true;
     break;
    }
    ancestor = ancestor.GetHierarchyParent();
   }
   if (nested)
    continue;
   InventoryLocation location = new InventoryLocation;
   if (!old.GetInventory().GetCurrentInventoryLocation(location))
   {
    DeleteStaged(staged);
    Reply(player, "Inventory is busy. Your inventory was kept.");
    return false;
   }
   oldItems.Insert(old);
   original.Insert(location);
  }
  array<EntityAI> moved = new array<EntityAI>;
  array<ref InventoryLocation> movedOriginal = new array<ref InventoryLocation>;
  for (int k = 0; k < oldItems.Count(); k++)
  {
   vector transform[4];
   oldItems[k].GetTransform(transform);
   transform[3] = stagingPosition;
   InventoryLocation ground = new InventoryLocation;
   ground.SetGround(oldItems[k], transform);
   if (!MoveTo(oldItems[k], ground))
   {
    DeleteStaged(staged);
    Restore(player, moved, movedOriginal);
    Reply(player, "Inventory is locked or busy. Old gear restored.");
    return false;
   }
   moved.Insert(oldItems[k]);
   movedOriginal.Insert(original[k]);
  }
  InventoryLocation handLocation = new InventoryLocation;
  handLocation.SetHands(player, gun);
  bool equipped = MoveTo(gun, handLocation);
  for (int a = 1; a < staged.Count() && equipped; a++)
  {
   InventoryLocation cargo = new InventoryLocation;
   if (!player.GetInventory().FindFreeLocationFor(staged[a], FindInventoryLocationType.CARGO, cargo))
    equipped = false;
   else if (!MoveTo(staged[a], cargo))
    equipped = false;
  }
  if (!equipped)
  {
   DeleteStaged(staged);
   Restore(player, moved, movedOriginal);
   Reply(player, "Not enough inventory space. Old gear restored.");
   return false;
  }
  // Commit only after every new item has successfully entered the inventory.
  DeleteStaged(oldItems);
  if (remember)
  {
   GRZSavedWeapon saved = new GRZSavedWeapon;
   saved.Weapon = weaponType;
   saved.Wrap = wrapType;
   if (!GRZWeaponStore.Save(player.GetIdentity(), saved))
   {
    Reply(player, "Loadout equipped, but saving for respawn failed. Check the server profile folder.");
    return true;
   }
  }
  Reply(player, "Loadout equipped. Your saved selection will be restored on respawn.");
  return true;
 }
};

modded class PlayerBase
{
 protected int m_GRZLoadoutLastRequest = -10000;
 protected int m_GRZLoadoutLastSave = -10000;

 override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
 {
  if (rpc_type == GRZLoadoutRPC.REQUEST || rpc_type == GRZLoadoutRPC.SAVE || rpc_type == GRZLoadoutRPC.SAVE_CLOTHING)
  {
   if (!g_Game.IsServer() || !sender || !GetIdentity())
    return;
   if (sender.GetId() != GetIdentity().GetId())
    return;
   GRZLoadoutServer.Init();
   int now = g_Game.GetTime();
   if (rpc_type == GRZLoadoutRPC.REQUEST)
   {
    if (now - m_GRZLoadoutLastRequest < 1000)
     return;
    m_GRZLoadoutLastRequest = now;
    ScriptRPC catalogueReply = new ScriptRPC;
    GRZLoadoutCatalogue personal = new GRZLoadoutCatalogue;
    personal.Guns = GRZLoadoutServer.Catalogue.Guns;
    personal.Wraps = GRZLoadoutServer.Catalogue.Wraps;
    personal.Characters = GRZLoadoutServer.Catalogue.Characters;
    personal.AmmoStacks = GRZLoadoutServer.Catalogue.AmmoStacks;
    personal.SavedOutfit = GRZOutfitStore.Load(sender);
    catalogueReply.Write(personal);
    catalogueReply.Send(this, GRZLoadoutRPC.CATALOGUE, true, sender);
    array<ref GRZLoadoutClothing> page = new array<ref GRZLoadoutClothing>;
    int clothesCount = GRZLoadoutServer.Catalogue.Clothing.Count();
    for (int clothingIndex = 0; clothingIndex < clothesCount; clothingIndex++)
    {
     page.Insert(GRZLoadoutServer.Catalogue.Clothing[clothingIndex]);
     if (page.Count() == 40 || clothingIndex == clothesCount - 1)
     {
      ScriptRPC clothingReply = new ScriptRPC;
      clothingReply.Write(page);
      clothingReply.Write(clothingIndex == clothesCount - 1);
      clothingReply.Send(this, GRZLoadoutRPC.CLOTHING_PAGE, true, sender);
      page.Clear();
     }
    }
    return;
   }
   int cooldown = Math.Clamp(GRZLoadoutServer.Settings.SaveCooldownSeconds, 1, 300) * 1000;
   if (now - m_GRZLoadoutLastSave < cooldown)
   {
    GRZLoadoutServer.Reply(this, "Please wait before saving another loadout.");
    return;
   }
   if (rpc_type == GRZLoadoutRPC.SAVE_CLOTHING)
   {
    array<string> clothing;
    if (!ctx.Read(clothing))
     return;
    string character;
    if (!ctx.Read(character))
     return;
    if (GRZLoadoutServer.Catalogue.Characters.Find(character) < 0)
     return;
    m_GRZLoadoutLastSave = now;
    if (GRZLoadoutClothingServer.Equip(this, clothing))
    {
     GRZSavedOutfit saved = GRZOutfitStore.Snapshot(this, character);
     if (GRZOutfitStore.Save(sender, saved))
      GRZLoadoutServer.Reply(this, "Outfit applied and saved for respawn. Character changes on next respawn.");
     else
      GRZLoadoutServer.Reply(this, "Clothing applied, but saving failed. Check the server profile folder.");
    }
    return;
   }
   string gun;
   string wrap;
   if (!ctx.Read(gun))
    return;
   if (!ctx.Read(wrap))
    return;
   m_GRZLoadoutLastSave = now;
   GRZLoadoutServer.Equip(this, gun, wrap);
   return;
  }
  if (rpc_type == GRZLoadoutRPC.CLOTHING_PAGE)
  {
   if (g_Game.IsServer() || this != g_Game.GetPlayer() || !GRZLoadoutClientData.Catalogue)
    return;
   array<ref GRZLoadoutClothing> receivedPage;
   bool lastPage;
   if (!ctx.Read(receivedPage))
    return;
   if (!ctx.Read(lastPage) || !receivedPage)
    return;
   foreach (GRZLoadoutClothing clothingEntry : receivedPage)
    GRZLoadoutClientData.Catalogue.Clothing.Insert(clothingEntry);
   if (lastPage)
    GRZLoadoutClientData.Changed.Invoke();
   return;
  }
  if (rpc_type == GRZLoadoutRPC.CATALOGUE || rpc_type == GRZLoadoutRPC.RESULT)
  {
   if (g_Game.IsServer() || this != g_Game.GetPlayer())
    return;
   if (rpc_type == GRZLoadoutRPC.CATALOGUE)
   {
    GRZLoadoutCatalogue received;
    if (ctx.Read(received) && received)
    {
     GRZLoadoutClientData.Catalogue = received;
     GRZLoadoutClientData.Changed.Invoke();
    }
   }
   else
   {
    string message;
    if (ctx.Read(message))
     GRZLoadoutClientData.Result.Invoke(message);
   }
   return;
  }
  super.OnRPC(sender, rpc_type, ctx);
 }
};
