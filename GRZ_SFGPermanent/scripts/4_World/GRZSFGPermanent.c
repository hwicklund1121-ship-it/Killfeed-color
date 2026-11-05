modded class sfg50_Base
{
 protected int m_GRZPermanentPatchAttempts;

 override void EEInit()
 {
  super.EEInit();
  GRZPermanentPatchStart();
 }

 override void EEOnAfterLoad()
 {
  super.EEOnAfterLoad();
  GRZPermanentPatchStart();
 }

 override void OnDebugSpawn()
 {
  super.OnDebugSpawn();
  GRZPermanentPatchStart();
 }

 protected void GRZPermanentPatchStart()
 {
  if (!g_Game.IsServer())
   return;
  m_GRZPermanentPatchAttempts = 0;
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(GRZPermanentPatchEnsure);
  GRZPermanentPatchEnsure();
 }

 void GRZPermanentPatchEnsure()
 {
  if (!g_Game.IsServer())
   return;
  int slot = InventorySlots.GetSlotIdFromString("UAV_SFG_Suppressor");
  EntityAI suppressor = FindAttachmentBySlotName("UAV_SFG_Suppressor");
  if (!suppressor && slot >= 0)
  {
   GetInventory().SetSlotLock(slot, false);
   suppressor = GetInventory().CreateAttachmentEx("UAV_SFG_Sup", slot);
  }
  if (suppressor)
  {
   GetInventory().SetSlotLock(slot, true);
   return;
  }
  m_GRZPermanentPatchAttempts++;
  if (m_GRZPermanentPatchAttempts < 10)
   g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GRZPermanentPatchEnsure, 250, false);
  else
   Error("[GRZ SFG Permanent] Cannot attach UAV_SFG_Sup to " + GetType());
 }

 override bool CanReleaseAttachment(EntityAI attachment)
 {
  if (attachment && attachment == FindAttachmentBySlotName("UAV_SFG_Suppressor"))
   return false;
  return super.CanReleaseAttachment(attachment);
 }

 override bool CanDisplayAttachmentSlot(int slot_id)
 {
  if (slot_id == InventorySlots.GetSlotIdFromString("UAV_SFG_Suppressor"))
   return false;
  return super.CanDisplayAttachmentSlot(slot_id);
 }

 override void EEDelete(EntityAI parent)
 {
  if (g_Game)
   g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(GRZPermanentPatchEnsure);
  super.EEDelete(parent);
 }
};
