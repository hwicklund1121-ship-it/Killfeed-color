class GRZLoadoutClothingServer
{
 static void Discover(GRZLoadoutCatalogue catalogue, bool allInstalled)
 {
  int count = g_Game.ConfigGetChildrenCount("CfgVehicles");
  for (int i = 0; i < count; i++)
  {
   string type;
   g_Game.ConfigGetChildName("CfgVehicles", i, type);
   if (g_Game.ConfigGetInt("CfgVehicles " + type + " scope") != 2)
    continue;
   if (!g_Game.IsKindOf(type, "Clothing") && !g_Game.IsKindOf(type, "Clothing_Base"))
    continue;
   if (!allInstalled)
   {
    string lower = type;
    lower.ToLower();
    if (lower.IndexOf("grz_") != 0 && lower.IndexOf("blake_") != 0)
     continue;
   }
   array<string> slots = new array<string>;
   g_Game.ConfigGetTextArray("CfgVehicles " + type + " inventorySlot", slots);
   foreach (string slot : slots)
   {
    if (GRZLoadoutClothingSlots.Names.Find(slot) < 0)
     continue;
    GRZLoadoutClothing clothing = new GRZLoadoutClothing;
    clothing.Type = type;
    clothing.Slot = slot;
    g_Game.ConfigGetText("CfgVehicles " + type + " displayName", clothing.Name);
    if (clothing.Name == "")
     clothing.Name = type;
    catalogue.Clothing.Insert(clothing);
   }
  }
  Print(string.Format("[GRZ Loadout] Registered %1 clothing choices.", catalogue.Clothing.Count()));
 }

 static bool Allowed(string type, string slot)
 {
  foreach (GRZLoadoutClothing item : GRZLoadoutServer.Catalogue.Clothing)
  {
   if (item.Type == type && item.Slot == slot)
    return true;
  }
  return false;
 }

 static bool Ground(EntityAI item, vector position)
 {
  vector transform[4];
  item.GetTransform(transform);
  transform[3] = position;
  InventoryLocation ground = new InventoryLocation;
  ground.SetGround(item, transform);
  return GRZLoadoutServer.MoveTo(item, ground);
 }

 static void Rollback(PlayerBase player, array<EntityAI> fresh, array<EntityAI> old, array<ref InventoryLocation> oldLocations, array<EntityAI> contents, array<ref InventoryLocation> contentLocations)
 {
  bool safeToDelete = true;
  for (int i = contents.Count() - 1; i >= 0; i--)
  {
   if (!GRZLoadoutServer.MoveTo(contents[i], contentLocations[i]))
   {
    if (!Ground(contents[i], player.GetPosition()))
     safeToDelete = false;
    Error("[GRZ Clothing] Another mod rejected a content rollback; preserving item.");
   }
  }
  if (safeToDelete)
   GRZLoadoutServer.DeleteStaged(fresh);
  else
  {
   // Never delete a garment if another mod trapped original contents inside it.
   foreach (EntityAI garment : fresh)
   {
    if (garment)
     Ground(garment, player.GetPosition());
   }
  }
  GRZLoadoutServer.Restore(player, old, oldLocations);
 }

 static void CollectChildren(EntityAI parent, array<EntityAI> children)
 {
  for (int a = 0; a < parent.GetInventory().AttachmentCount(); a++)
  {
   EntityAI attachment = parent.GetInventory().GetAttachmentFromIndex(a);
   if (attachment)
    children.Insert(attachment);
  }
  CargoBase cargo = parent.GetInventory().GetCargo();
  if (cargo)
  {
   for (int c = 0; c < cargo.GetItemCount(); c++)
    children.Insert(cargo.GetItem(c));
  }
 }

 static bool TransferChild(PlayerBase player, EntityAI child, EntityAI garment)
 {
  InventoryLocation destination = new InventoryLocation;
  int flags = FindInventoryLocationType.ATTACHMENT | FindInventoryLocationType.CARGO;
  if (garment && garment.GetInventory().FindFreeLocationFor(child, flags, destination))
  {
   if (GRZLoadoutServer.MoveTo(child, destination))
    return true;
  }
  // Other worn clothing may hold overflow; original items are never discarded.
  if (player.GetInventory().FindFreeLocationFor(child, FindInventoryLocationType.CARGO, destination))
   return GRZLoadoutServer.MoveTo(child, destination);
  return false;
 }

 static bool Equip(PlayerBase player, array<string> choices)
 {
  if (!GRZLoadoutServer.Settings.Enabled || !player.IsAlive() || player.IsUnconscious())
  {
   GRZLoadoutServer.Reply(player, "Clothing unavailable right now.");
   return false;
  }
  if (player.IsRestrained() || player.GetCommand_Vehicle() || player.IsRaised())
  {
   GRZLoadoutServer.Reply(player, "Lower your weapon and leave the vehicle first.");
   return false;
  }
  if (!choices || choices.Count() != GRZLoadoutClothingSlots.Names.Count())
   return false;
  array<EntityAI> fresh = new array<EntityAI>;
  array<EntityAI> old = new array<EntityAI>;
  array<ref InventoryLocation> original = new array<ref InventoryLocation>;
  array<int> selectedSlots = new array<int>;
  array<EntityAI> contents = new array<EntityAI>;
  array<ref InventoryLocation> contentLocations = new array<ref InventoryLocation>;
  vector staging = player.GetPosition() + "0 -1000 0";
  int createFlags = ECE_KEEPHEIGHT | ECE_NOSURFACEALIGN | ECE_NOLIFETIME;
  for (int i = 0; i < choices.Count(); i++)
  {
   if (choices[i] == "")
    continue;
   string slotName = GRZLoadoutClothingSlots.Names[i];
   if (choices[i] != "__REMOVE__" && !Allowed(choices[i], slotName))
   {
    GRZLoadoutServer.DeleteStaged(fresh);
    GRZLoadoutServer.Reply(player, "Clothing selection is not allowed by the server.");
    return false;
   }
   EntityAI current = player.FindAttachmentBySlotName(slotName);
   if (current && current.GetType() == choices[i])
    continue;
   if (choices[i] == "__REMOVE__")
   {
    if (current)
    {
     fresh.Insert(null);
     selectedSlots.Insert(InventorySlots.GetSlotIdFromString(slotName));
    }
    continue;
   }
   EntityAI created = EntityAI.Cast(g_Game.CreateObjectEx(choices[i], staging, createFlags));
   if (!created || !created.IsClothing())
   {
    if (created)
     g_Game.ObjectDelete(created);
    GRZLoadoutServer.DeleteStaged(fresh);
    GRZLoadoutServer.Reply(player, "Clothing creation failed. Existing gear was kept.");
    return false;
   }
   fresh.Insert(created);
   selectedSlots.Insert(InventorySlots.GetSlotIdFromString(slotName));
  }
  if (fresh.Count() == 0)
  {
   return true;
  }
  // Stage all replaced garments with their full contents still inside them.
  for (int j = 0; j < selectedSlots.Count(); j++)
  {
   EntityAI previous = player.GetInventory().FindAttachment(selectedSlots[j]);
   if (!previous)
    continue;
   InventoryLocation previousLocation = new InventoryLocation;
   if (!previous.GetInventory().GetCurrentInventoryLocation(previousLocation) || !Ground(previous, staging))
   {
    Rollback(player, fresh, old, original, contents, contentLocations);
    GRZLoadoutServer.Reply(player, "Clothing is locked or busy. Existing gear restored.");
    return false;
   }
   old.Insert(previous);
   original.Insert(previousLocation);
  }
  for (int k = 0; k < fresh.Count(); k++)
  {
   if (!fresh[k])
    continue;
   InventoryLocation wearable = new InventoryLocation;
   wearable.SetAttachment(player, fresh[k], selectedSlots[k]);
   if (!GRZLoadoutServer.MoveTo(fresh[k], wearable))
   {
    Rollback(player, fresh, old, original, contents, contentLocations);
    GRZLoadoutServer.Reply(player, "That clothing combination cannot be worn together. Existing gear restored.");
    return false;
   }
  }
  // Preserve every direct child; nested containers keep their own full contents.
  for (int g = 0; g < old.Count(); g++)
  {
   array<EntityAI> children = new array<EntityAI>;
   CollectChildren(old[g], children);
   EntityAI replacement = player.GetInventory().FindAttachment(original[g].GetSlot());
   foreach (EntityAI child : children)
   {
    InventoryLocation previousChildLocation = new InventoryLocation;
    if (!child.GetInventory().GetCurrentInventoryLocation(previousChildLocation))
    {
     Rollback(player, fresh, old, original, contents, contentLocations);
     GRZLoadoutServer.Reply(player, "Inventory is busy. Existing gear restored.");
     return false;
    }
    if (!TransferChild(player, child, replacement))
    {
     Rollback(player, fresh, old, original, contents, contentLocations);
     GRZLoadoutServer.Reply(player, "Items do not fit in the new outfit. Existing gear restored.");
     return false;
    }
    contents.Insert(child);
    contentLocations.Insert(previousChildLocation);
   }
  }
  GRZLoadoutServer.DeleteStaged(old);
  return true;
 }
};
