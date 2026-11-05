class GRZOutfitStore
{
 static string PathFor(PlayerIdentity identity)
 {
  MakeDirectory("$profile:GRZ_Loadout");
  MakeDirectory("$profile:GRZ_Loadout/players");
  return "$profile:GRZ_Loadout/players/" + identity.GetPlainId() + ".json";
 }

 static bool Valid(GRZSavedOutfit saved)
 {
  if (!saved || !saved.Choices)
   return false;
  if (saved.Choices.Count() != GRZLoadoutClothingSlots.Names.Count())
   return false;
  return g_Game.ListAvailableCharacters().Find(saved.Character) >= 0;
 }

 static GRZSavedOutfit Load(PlayerIdentity identity)
 {
  if (!identity)
   return null;
  string path = PathFor(identity);
  GRZSavedOutfit saved;
  string error;
  if (JsonFileLoader<GRZSavedOutfit>.LoadFile(path, saved, error) && Valid(saved))
   return saved;
  if (JsonFileLoader<GRZSavedOutfit>.LoadFile(path + ".backup", saved, error) && Valid(saved))
   return saved;
  return null;
 }

 static bool Same(GRZSavedOutfit a, GRZSavedOutfit b)
 {
  if (!Valid(a) || !Valid(b) || a.Character != b.Character)
   return false;
  for (int i = 0; i < a.Choices.Count(); i++)
  {
   if (a.Choices[i] != b.Choices[i])
    return false;
  }
  return true;
 }

 static bool Save(PlayerIdentity identity, GRZSavedOutfit saved)
 {
  if (!identity || !Valid(saved))
   return false;
  string path = PathFor(identity);
  string error;
  if (!JsonFileLoader<GRZSavedOutfit>.SaveFile(path + ".pending", saved, error))
   return false;
  GRZSavedOutfit verify;
  if (!JsonFileLoader<GRZSavedOutfit>.LoadFile(path + ".pending", verify, error) || !Same(saved, verify))
   return false;
  GRZSavedOutfit previous;
  if (JsonFileLoader<GRZSavedOutfit>.LoadFile(path, previous, error) && Valid(previous))
  {
   if (!CopyFile(path, path + ".backup"))
    return false;
  }
  if (!CopyFile(path + ".pending", path))
  {
   if (FileExist(path + ".backup"))
    CopyFile(path + ".backup", path);
   return false;
  }
  if (!JsonFileLoader<GRZSavedOutfit>.LoadFile(path, verify, error) || !Same(saved, verify))
  {
   if (FileExist(path + ".backup"))
    CopyFile(path + ".backup", path);
   return false;
  }
  DeleteFile(path + ".pending");
  return true;
 }

 static GRZSavedOutfit Snapshot(PlayerBase player, string character)
 {
  GRZSavedOutfit saved = new GRZSavedOutfit;
  saved.Character = character;
  foreach (string slot : GRZLoadoutClothingSlots.Names)
  {
   EntityAI clothing = player.FindAttachmentBySlotName(slot);
   string type = "__REMOVE__";
   if (clothing)
    type = clothing.GetType();
   saved.Choices.Insert(type);
  }
  return saved;
 }
};
