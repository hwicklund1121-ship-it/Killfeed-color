class GRZVipAllowlist
{
 ref array<string> AllowedSteamIds = new array<string>;
};
class GRZVipReskinSettings
{
 ref array<string> ExtraPaidGuns = new array<string>;
 ref array<string> FreeGuns = new array<string>;
 ref array<string> ExtraPaidWraps = new array<string>;
};
class GRZVipLoadoutPolicy
{
 protected static ref GRZVipReskinSettings s_Settings;
 protected static ref GRZVipAllowlist s_Access;
 protected static int s_AccessAt = -10000;
 protected static ref array<string> s_Suppressors;
 protected static ref map<string, bool> s_Paid = new map<string, bool>;
 static void Init()
 {
  if (s_Settings) return;
  s_Settings = new GRZVipReskinSettings;
  MakeDirectory("$profile:GRZ_Loadout");
  string path = "$profile:GRZ_Loadout/vip-reskins.json";
  string error;
  if (FileExist(path))
  {
   GRZVipReskinSettings loaded;
   if (JsonFileLoader<GRZVipReskinSettings>.LoadFile(path, loaded, error) && loaded && loaded.ExtraPaidGuns && loaded.FreeGuns && loaded.ExtraPaidWraps)
    s_Settings = loaded;
   else Print("[GRZ VIP] Invalid vip-reskins.json; using automatic GRZ reskin detection.");
  }
  else JsonFileLoader<GRZVipReskinSettings>.SaveFile(path, s_Settings, error);
 }
 static bool Allowed(PlayerIdentity identity)
 {
  if (!identity || !g_Game.IsServer()) return false;
  if (g_Game.GetTime() - s_AccessAt >= 5000)
  {
   s_AccessAt = g_Game.GetTime();
   s_Access = null;
   GRZVipAllowlist loaded;
   string error;
   if (JsonFileLoader<GRZVipAllowlist>.LoadFile("$profile:GRZ_Killfeed/config.json", loaded, error) && loaded && loaded.AllowedSteamIds)
    s_Access = loaded;
  }
  return s_Access && s_Access.AllowedSteamIds.Find(identity.GetPlainId()) >= 0;
 }
 static bool PaidGun(string type)
 {
  Init();
  bool cached;
  if (s_Paid.Find(type, cached)) return cached;
  bool paid = s_Settings.ExtraPaidGuns.Find(type) >= 0 || g_Game.ConfigGetInt("CfgWeapons " + type + " GRZVipOnly") == 1;
  string lower = type; lower.ToLower();
  if (lower.IndexOf("grz_") == 0 || lower.IndexOf("rvk_") == 0 || lower.IndexOf("xims_") == 0 || lower.IndexOf("blake_") == 0) paid = true;
  array<string> textures = new array<string>;
  g_Game.ConfigGetTextArray("CfgWeapons " + type + " hiddenSelectionsTextures", textures);
  foreach (string texture : textures)
  {
   texture.ToLower();
   if (texture.Contains("grz_") || texture.Contains("gunrunnerz") || texture.Contains("blake_")) paid = true;
  }
  if (s_Settings.FreeGuns.Find(type) >= 0) paid = false;
  s_Paid.Set(type, paid);
  return paid;
 }
 static bool PaidWrap(string type)
 {
  if (type == "") return false;
  Init();
  string lower = type; lower.ToLower();
  return s_Settings.ExtraPaidWraps.Find(type) >= 0 || g_Game.ConfigGetInt("CfgVehicles " + type + " GRZVipOnly") == 1 || lower.Contains("animated");
 }
 static bool CanUseWeapon(PlayerBase owner, EntityAI gun)
 {
  if (!owner || !owner.GetIdentity() || !gun) return true;
  EntityAI wrap = gun.FindAttachmentBySlotName("weaponWrap");
  bool paid = PaidGun(gun.GetType());
  if (wrap && PaidWrap(wrap.GetType())) paid = true;
  return !paid || Allowed(owner.GetIdentity());
 }
 static bool EnsureSuppressor(EntityAI gun)
 {
  if (!gun) return false;
  if (gun.IsKindOf("sfg50_Base")) return true; // Permanent SFG setup is handled separately.
  array<string> mounts = new array<string>;
  g_Game.ConfigGetTextArray("CfgWeapons " + gun.GetType() + " attachments", mounts);
  foreach (string mount : mounts)
  {
   EntityAI existing = gun.FindAttachmentBySlotName(mount);
   if (existing && existing.IsKindOf("ItemSuppressor")) return true;
  }
  if (!s_Suppressors)
  {
   s_Suppressors = new array<string>;
   int suppressorCount = g_Game.ConfigGetChildrenCount("CfgVehicles");
   for (int candidateIndex = 0; candidateIndex < suppressorCount; candidateIndex++)
   {
    string candidate;
    g_Game.ConfigGetChildName("CfgVehicles", candidateIndex, candidate);
    if (g_Game.ConfigGetInt("CfgVehicles " + candidate + " scope") == 2 && g_Game.IsKindOf(candidate, "ItemSuppressor")) s_Suppressors.Insert(candidate);
   }
  }
  bool compatible;
  int count = s_Suppressors.Count();
  for (int i = 0; i < count; i++)
  {
   string type = s_Suppressors[i];
   array<string> slots = new array<string>;
   g_Game.ConfigGetTextArray("CfgVehicles " + type + " inventorySlot", slots);
   foreach (string slot : slots)
   {
    if (mounts.Find(slot) < 0) continue;
    compatible = true;
    if (gun.GetInventory().CreateAttachmentEx(type, InventorySlots.GetSlotIdFromString(slot))) return true;
   }
  }
  return !compatible; // Guns without a compatible suppressor remain valid.
 }
};
modded class PlayerBase
{
 override bool CanReceiveItemIntoHands(EntityAI item_to_hands)
 {
  if (g_Game.IsServer() && item_to_hands && Weapon_Base.Cast(item_to_hands) && !GRZVipLoadoutPolicy.CanUseWeapon(this, item_to_hands)) return false;
  return super.CanReceiveItemIntoHands(item_to_hands);
 }
};
modded class Weapon_Base
{
 override bool CanFire()
 {
  if (g_Game.IsServer())
  {
   PlayerBase owner = PlayerBase.Cast(GetHierarchyRootPlayer());
   if (!GRZVipLoadoutPolicy.CanUseWeapon(owner, this)) return false;
  }
  return super.CanFire();
 }
};
