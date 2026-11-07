class GRZLoadoutClothingServer
{
 // Register our kits first so they arrive in the first clothing page.
 static ref array<string> Preferred = {"GRZ_SurgicalGloves", "GRZ_TShirt", "GRZ_CowboyHat", "GRZ_Jeans", "GRZ_Bandana", "GRZ_Sneakers", "XIMS_Sneakers", "XIMS_JeanShorts", "XIMS_LayeredShirt", "XIMS_CanvasBag"};
 static string CanonicalSlot(string value)
 {
  value.TrimInPlace();
  value.ToLower();
  foreach (string candidate : GRZLoadoutClothingSlots.Names)
  {
   string lower = candidate;
   lower.ToLower();
   if (value == lower) return candidate;
  }
  return "";
 }
 static void ReadSlots(string type, array<string> slots)
 {
  string path = "CfgVehicles " + type + " inventorySlot";
  array<string> configured = new array<string>;
  int kind = g_Game.ConfigGetType(path);
  if (kind == CT_ARRAY) g_Game.ConfigGetTextArray(path, configured);
  else if (kind == CT_STRING)
  {
   string single;
   g_Game.ConfigGetText(path, single);
   configured.Insert(single);
  }
  foreach (string entry : configured)
  {
   string canonical = CanonicalSlot(entry);
   if (canonical != "" && slots.Find(canonical) < 0) slots.Insert(canonical);
  }
 }
 static bool Register(GRZLoadoutCatalogue catalogue, string type, out string reason)
 {
  if (!g_Game.ConfigIsExisting("CfgVehicles " + type)) { reason = "MISSING ON SERVER (check server PBO and active -mod folders)"; return false; }
  if (g_Game.ConfigGetInt("CfgVehicles " + type + " scope") != 2) { reason = "scope is not public"; return false; }
  if (!g_Game.IsKindOf(type, "Clothing") && !g_Game.IsKindOf(type, "Clothing_Base")) { reason = "not clothing (check original base addon dependencies)"; return false; }
  array<string> slots = new array<string>;
  ReadSlots(type, slots);
  if (slots.Count() == 0) { reason = "no supported inventorySlot; check final config overrides"; return false; }
  reason = "REGISTERED";
  foreach (string slot : slots)
  {
   GRZLoadoutClothing clothing = new GRZLoadoutClothing;
   clothing.Type = type;
   clothing.Slot = slot;
   g_Game.ConfigGetText("CfgVehicles " + type + " displayName", clothing.Name);
   if (clothing.Name == "") clothing.Name = type;
   catalogue.Clothing.Insert(clothing);
   reason += " " + slot;
  }
  return true;
 }
 static void Discover(GRZLoadoutCatalogue catalogue, bool allInstalled)
 {
  map<string, bool> seen = new map<string, bool>;
  string report = "GRZ clothing registration (server-side)\n";
  foreach (string preferred : Preferred)
  {
   string preferredReason;
   if (Register(catalogue, preferred, preferredReason))
   {
    string preferredKey = preferred;
    preferredKey.ToLower();
    seen.Set(preferredKey, true);
   }
   report += preferred + ": " + preferredReason + "\n";
   Print("[GRZ Loadout] " + preferred + ": " + preferredReason);
  }
  int count = g_Game.ConfigGetChildrenCount("CfgVehicles");
  for (int i = 0; i < count; i++)
  {
   string type;
   g_Game.ConfigGetChildName("CfgVehicles", i, type);
   string lower = type;
   lower.ToLower();
   if (seen.Contains(lower)) continue;
   if (!allInstalled)
   {
    if (lower.IndexOf("grz_") != 0 && lower.IndexOf("blake_") != 0 && lower.IndexOf("xims_") != 0) continue;
   }
   string reason;
   if (Register(catalogue, type, reason)) seen.Set(lower, true);
  }
  FileHandle file = OpenFile("$profile:GRZ_Loadout/clothing-registration.txt", FileMode.WRITE);
  if (file) { FPrint(file, report); CloseFile(file); }
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
  // Check the complete selection before staging or moving any inventory.
  foreach (string requested : choices)
  {
   if (requested != "" && requested != "__REMOVE__" && GRZVipLoadoutPolicy.PaidClothing(requested) && !GRZVipLoadoutPolicy.Allowed(player.GetIdentity()))
   {
    GRZLoadoutServer.Reply(player, "Animated clothing requires VIP access.");
    return false;
   }
  }
  string auditClothes;
  bool auditVip;
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
   if (current)
   {
    auditClothes += slotName + ": " + current.GetType() + " -> " + choices[i] + " | ";
    if (GRZVipLoadoutPolicy.PaidClothing(current.GetType())) auditVip = true;
   }
   else auditClothes += slotName + ": empty -> " + choices[i] + " | ";
   if (GRZVipLoadoutPolicy.PaidClothing(choices[i])) auditVip = true;
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
  if (selectedSlots.Count() > 0) GRZAudit.Record("clothing_change", player.GetIdentity(), auditClothes, null, auditVip);
  return true;
 }
};
