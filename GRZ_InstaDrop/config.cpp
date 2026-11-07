class CfgPatches
{
 class GRZ_InstaDrop
 {
  units[] = {};
  weapons[] = {};
  requiredVersion = 0.1;
  requiredAddons[] = {"DZ_Scripts"};
 };
};
class CfgMods
{
 class GRZ_InstaDrop
 {
  dir = "GRZ_InstaDrop";
  name = "GRZ InstaDrop Prototype";
  version = "0.1.5";
  author = "GRZ";
  type = "mod";
  dependencies[] = {"World"};
  class defs
  {
   class worldScriptModule
   {
    value = "";
    files[] = {"GRZ_InstaDrop/scripts/4_World"};
   };
  };
 };
};
