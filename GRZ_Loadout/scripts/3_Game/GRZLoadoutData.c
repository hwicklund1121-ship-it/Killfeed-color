class GRZLoadoutRPC
{
 static const int REQUEST = 763280;
 static const int CATALOGUE = 763281;
 static const int SAVE = 763282;
 static const int RESULT = 763283;
 static const int MENU = 763284;
 static const int SAVE_CLOTHING = 763285;
 static const int CLOTHING_PAGE = 763286;
 static const int VIP_GEAR_APPLY = 763287;
};

class GRZLoadoutGun
{
 string Type;
 string Name;
 string Ammo;
 string MagazineType;
 string ScopeType;
 bool VipOnly;
 int Category;
};

class GRZLoadoutClothing
{
 string Type;
 string Name;
 string Slot;
};

class GRZLoadoutClothingSlots
{
 static ref array<string> Names = {"Headgear", "Mask", "Eyewear", "Body", "Vest", "Gloves", "Legs", "Feet", "Back", "Hips", "Armband"};
};

class GRZSavedOutfit
{
 string Character;
 ref array<string> Choices = new array<string>;
};

class GRZLoadoutCatalogue
{
 bool VipAuthorized;
 ref array<ref GRZLoadoutClothing> VipClothing = new array<ref GRZLoadoutClothing>;
 ref array<string> VipWraps = new array<string>;
 ref array<ref GRZLoadoutGun> VipGuns = new array<ref GRZLoadoutGun>;
 ref array<ref GRZLoadoutGun> Guns = new array<ref GRZLoadoutGun>;
 ref array<string> Wraps = new array<string>;
 ref array<ref GRZLoadoutClothing> Clothing = new array<ref GRZLoadoutClothing>;
 ref array<string> Characters = new array<string>;
 ref GRZSavedOutfit SavedOutfit;
 int AmmoStacks = 10;
 
};

class GRZLoadoutSettings
{
 bool Enabled = true;
 bool AutoDiscover = true;
 bool AllInstalledClothing = true;
 int AmmoStacks = 10;
 
 int SaveCooldownSeconds = 5;
 // Add explicit entries here for weapon packs whose calibre names are unusual.
 // Category: 0 = 20mm, 1 = .50 cal, 2 = .408.
 ref array<ref GRZLoadoutGun> Guns = new array<ref GRZLoadoutGun>;
};

class GRZLoadoutClientData
{
 static ref GRZLoadoutCatalogue Catalogue;
 static ref ScriptInvoker Changed = new ScriptInvoker;
 static ref ScriptInvoker Result = new ScriptInvoker;
};

class GRZSavedWeapon
{
 string Weapon;
 string Wrap;
};

class GRZRespawnChoiceRPC
{
 static const int CHOOSE = 763630;
 static const int ACK = 763631;
};
