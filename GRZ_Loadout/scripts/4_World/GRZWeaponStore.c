class GRZWeaponStore
{
 static string PathFor(PlayerIdentity identity)
 {
  MakeDirectory("$profile:GRZ_Loadout");
  MakeDirectory("$profile:GRZ_Loadout/weapons");
  return "$profile:GRZ_Loadout/weapons/" + identity.GetPlainId() + ".json";
 }

 static bool Valid(GRZSavedWeapon saved)
 {
  return saved && saved.Weapon != "";
 }

 static GRZSavedWeapon Load(PlayerIdentity identity)
 {
  if (!identity)
   return null;
  string path = PathFor(identity);
  GRZSavedWeapon saved;
  string error;
  if (JsonFileLoader<GRZSavedWeapon>.LoadFile(path, saved, error) && Valid(saved))
   return saved;
  if (JsonFileLoader<GRZSavedWeapon>.LoadFile(path + ".backup", saved, error) && Valid(saved))
   return saved;
  return null;
 }

 static bool Same(GRZSavedWeapon a, GRZSavedWeapon b)
 {
  return Valid(a) && Valid(b) && a.Weapon == b.Weapon && a.Wrap == b.Wrap;
 }

 static bool Save(PlayerIdentity identity, GRZSavedWeapon saved)
 {
  if (!identity || !Valid(saved))
   return false;
  string path = PathFor(identity);
  string error;
  if (!JsonFileLoader<GRZSavedWeapon>.SaveFile(path + ".pending", saved, error))
   return false;
  GRZSavedWeapon verify;
  if (!JsonFileLoader<GRZSavedWeapon>.LoadFile(path + ".pending", verify, error) || !Same(saved, verify))
   return false;
  GRZSavedWeapon previous;
  if (JsonFileLoader<GRZSavedWeapon>.LoadFile(path, previous, error) && Valid(previous))
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
  if (!JsonFileLoader<GRZSavedWeapon>.LoadFile(path, verify, error) || !Same(saved, verify))
  {
   if (FileExist(path + ".backup"))
    CopyFile(path + ".backup", path);
   return false;
  }
  DeleteFile(path + ".pending");
  return true;
 }

};
