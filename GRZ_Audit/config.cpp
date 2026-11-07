class CfgPatches { class GRZ_Audit { units[] = {}; weapons[] = {}; requiredVersion = 0.1; requiredAddons[] = {"DZ_Data","DZ_Scripts"}; }; };
class CfgMods { class GRZ_Audit { name = "GRZ Audit Logs"; author = "Blake"; version = "0.1.0"; type = "mod"; dependencies[] = {"World"}; class defs { class worldScriptModule { value = ""; files[] = {"GRZ_Audit/scripts/4_World"}; }; }; }; };
