modded class MissionServer
{
 override PlayerBase CreateCharacter(PlayerIdentity identity, vector pos, ParamsReadContext ctx, string characterName)
 {
  GRZSavedOutfit saved = GRZOutfitStore.Load(identity);
  if (saved)
   characterName = saved.Character;
  return super.CreateCharacter(identity, pos, ctx, characterName);
 }

 override PlayerBase OnClientNewEvent(PlayerIdentity identity, vector pos, ParamsReadContext ctx)
 {
  PlayerBase player = super.OnClientNewEvent(identity, pos, ctx);
  if (player)
   g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GRZApplySavedLoadout, 500, false, player);
  return player;
 }

 void GRZApplySavedLoadout(PlayerBase player)
 {
  if (!player || !player.GetIdentity() || !player.IsAlive())
   return;
  // Restore clothing/cargo space first, then run the existing transactional
  // gun grant. Reconnects use ClientReady and do not receive duplicate gear.
  GRZRemoveStarterItems(player);
  GRZApplySavedClothing(player);
  if (GRZDuelServer.Spawned(player)) return;
  GRZSavedWeapon weapon = GRZWeaponStore.Load(player.GetIdentity());
  if (weapon && !GRZLoadoutServer.Equip(player, weapon.Weapon, weapon.Wrap, false))
   Print("[GRZ Loadout] Saved weapon could not be restored; spawn gear preserved.");
 }

 void GRZRemoveStarterItems(PlayerBase player)
 {
  // Called only for a newly created character, after native starter gear.
  // Clear these before dressing/granting the saved loadout to free cargo.
  array<EntityAI> items = new array<EntityAI>;
  player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
  for (int i = items.Count() - 1; i >= 0; i--)
  {
   EntityAI item = items[i];
   if (!item || item == player)
    continue;
   if (item.IsKindOf("SodaCan_ColorBase") || item.IsKindOf("Chemlight_ColorBase") || item.IsKindOf("Pear") || item.IsKindOf("Apple") || item.IsKindOf("Plum"))
    g_Game.ObjectDelete(item);
  }
 }

 void GRZApplySavedClothing(PlayerBase player)
 {
  if (!player || !player.GetIdentity() || !player.IsAlive())
   return;
  GRZSavedOutfit saved = GRZOutfitStore.Load(player.GetIdentity());
  if (!saved)
   return;
  GRZLoadoutServer.Init();
  // Missing/unloaded clothing mods never cause an inventory clear on respawn.
  if (!GRZLoadoutClothingServer.Equip(player, saved.Choices))
   Print("[GRZ Clothing] Saved outfit could not be equipped; spawn gear preserved.");
 }
};
