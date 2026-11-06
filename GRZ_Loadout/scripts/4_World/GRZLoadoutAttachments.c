class GRZLoadoutAttachments
{
 static bool AttachScope(EntityAI gun, string scopeType)
 {
  if (!gun) return false;
  if (scopeType == "" && gun.IsKindOf("sfg50_Base")) scopeType = "HuntingOptic";
  if (scopeType == "") return true;
  array<string> slots = new array<string>;
  g_Game.ConfigGetTextArray("CfgVehicles " + scopeType + " inventorySlot", slots);
  foreach (string slot : slots)
  {
   EntityAI existing = gun.FindAttachmentBySlotName(slot);
   if (existing && existing.IsKindOf(scopeType)) return true;
  }
  return gun.GetInventory().CreateAttachment(scopeType) != null;
 }

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
