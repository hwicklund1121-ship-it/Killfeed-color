class GRZDuelKits
{
 static void Attachments(EntityAI parent, int parentIndex, GRZDuelKit kit, int depth = 0)
 {
  if (depth > 8 || kit.Attachments.Count() >= 64) return;
  for (int i = 0; i < parent.GetInventory().AttachmentCount(); i++)
  {
   EntityAI item = parent.GetInventory().GetAttachmentFromIndex(i);
   if (!item || Magazine.Cast(item)) continue;
   InventoryLocation location = new InventoryLocation;
   if (!item.GetInventory().GetCurrentInventoryLocation(location)) continue;
   GRZDuelAttachment row = new GRZDuelAttachment;
   row.Type = item.GetType();
   row.Parent = parentIndex;
   row.Slot = location.GetSlot();
   int index = kit.Attachments.Count();
   kit.Attachments.Insert(row);
   Attachments(item, index, kit, depth + 1);
  }
 }
 static GRZDuelKit Capture(PlayerBase player)
 {
  if (!player) return null;
  Weapon_Base weapon = Weapon_Base.Cast(player.GetHumanInventory().GetEntityInHands());
  if (!weapon || weapon.GetMuzzleCount() != 1) return null;
  GRZDuelKit kit = new GRZDuelKit;
  kit.Gun = new GRZLoadoutGun;
  kit.Gun.Type = weapon.GetType();
  g_Game.ConfigGetText("CfgWeapons " + kit.Gun.Type + " displayName", kit.Gun.Name);
  if (kit.Gun.Name == "") kit.Gun.Name = kit.Gun.Type;
  Magazine loaded = weapon.GetMagazine(0);
  string bullet = weapon.GetChamberAmmoTypeName(0);
  if (loaded)
  {
   kit.Gun.MagazineType = loaded.GetType();
   g_Game.ConfigGetText("CfgMagazines " + loaded.GetType() + " ammo", bullet);
  }
  array<string> chamber = new array<string>;
  g_Game.ConfigGetTextArray("CfgWeapons " + kit.Gun.Type + " chamberableFrom", chamber);
  foreach (string candidate : chamber)
  {
   string candidateBullet;
   g_Game.ConfigGetText("CfgMagazines " + candidate + " ammo", candidateBullet);
   if (g_Game.IsKindOf(candidate, "Ammunition_Base") && (bullet == "" || bullet == candidateBullet))
   {
    kit.Gun.Ammo = candidate;
    bullet = candidateBullet;
    break;
   }
  }
  if (kit.Gun.Ammo == "" && bullet != "")
  {
   int count = g_Game.ConfigGetChildrenCount("CfgMagazines");
   for (int a = 0; a < count; a++)
   {
    string pile;
    string pileBullet;
    g_Game.ConfigGetChildName("CfgMagazines", a, pile);
    if (!g_Game.IsKindOf(pile, "Ammunition_Base")) continue;
    g_Game.ConfigGetText("CfgMagazines " + pile + " ammo", pileBullet);
    if (pileBullet == bullet && g_Game.ConfigGetInt("CfgMagazines " + pile + " scope") == 2)
    {
     kit.Gun.Ammo = pile;
     break;
    }
   }
  }
  if (kit.Gun.Ammo == "") return null;
  if (!loaded)
  {
   array<string> magazines = new array<string>;
   g_Game.ConfigGetTextArray("CfgWeapons " + kit.Gun.Type + " magazines", magazines);
   foreach (string magazineType : magazines)
   {
    string magazineBullet;
    g_Game.ConfigGetText("CfgMagazines " + magazineType + " ammo", magazineBullet);
    if (magazineBullet == bullet)
    {
     kit.Gun.MagazineType = magazineType;
     break;
    }
   }
  }
  Attachments(weapon, -1, kit);
  return kit;
 }
 static bool AddAttachments(Weapon_Base weapon, GRZDuelKit kit)
 {
  array<EntityAI> nodes = new array<EntityAI>;
  for (int i = 0; i < kit.Attachments.Count(); i++)
  {
   GRZDuelAttachment row = kit.Attachments[i];
   EntityAI parent = weapon;
   if (row.Parent >= 0)
   {
    if (row.Parent >= nodes.Count()) return false;
    parent = nodes[row.Parent];
   }
   EntityAI item = parent.GetInventory().FindAttachment(row.Slot);
   if (!item) item = parent.GetInventory().CreateAttachmentEx(row.Type, row.Slot);
   if (!item || item.GetType() != row.Type) return false;
   nodes.Insert(item);
  }
  return true;
 }
};
