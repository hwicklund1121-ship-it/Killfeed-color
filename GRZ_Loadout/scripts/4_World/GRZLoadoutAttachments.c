class GRZLoadoutAttachments
{
 static bool EnsurePermanentSFGSuppressor(EntityAI gun, bool lockSlot)
 {
  if (!gun || !gun.IsKindOf("sfg50_Base"))
   return true;
  if (!g_Game.ConfigIsExisting("CfgVehicles UAV_SFG_Sup"))
   return false;
  int slot = InventorySlots.GetSlotIdFromString("UAV_SFG_Suppressor");
  if (slot < 0)
   return false;
  EntityAI suppressor = gun.FindAttachmentBySlotName("UAV_SFG_Suppressor");
  if (!suppressor)
  {
   gun.GetInventory().SetSlotLock(slot, false);
   suppressor = gun.GetInventory().CreateAttachmentEx("UAV_SFG_Sup", slot);
  }
  if (!suppressor)
   return false;
  if (lockSlot)
   gun.GetInventory().SetSlotLock(slot, true);
  return true;
 }
};
