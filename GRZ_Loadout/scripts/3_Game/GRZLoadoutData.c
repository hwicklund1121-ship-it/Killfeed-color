class GRZLoadoutRPC
{
 static const int REQUEST = 763280;
 static const int CATALOGUE = 763281;
 static const int SAVE = 763282;
 static const int RESULT = 763283;
 static const int MENU = 763284;
};

class GRZLoadoutGun
{
 string Type;
 string Name;
 string Ammo;
 string MagazineType;
 string ScopeType;
 int Category;
};

class GRZLoadoutCatalogue
{
 ref array<ref GRZLoadoutGun> Guns = new array<ref GRZLoadoutGun>;
 ref array<string> Wraps = new array<string>;
 int AmmoStacks = 10;
 
};

class GRZLoadoutSettings
{
 bool Enabled = true;
 bool AutoDiscover = true;
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
