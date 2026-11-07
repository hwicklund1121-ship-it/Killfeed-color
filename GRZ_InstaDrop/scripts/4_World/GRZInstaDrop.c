// GRZ stance support paired with the v0.1.9 modified vanilla drop graph.
// No LUGU scripts/graphs or forced player positions.
class GRZInstaDropTransitions
{
 bool OverrideTransitionSpeeds = false;
 float DropSpeed = 2.0;
 float RecoverySpeed = 1.6;
 ref array<string> Drop = new array<string>;
 ref array<string> Recover = new array<string>;
};
class GRZInstaDrop
{
 protected static bool Applied;
 protected static bool Ready;
 static bool IsEnabled()
 {
  return Ready;
 }
 static void Apply()
 {
  if (Applied) return;
  Applied = true;
  if (GetGame().ConfigIsExisting("CfgPatches LUGUINSTADROPFIX"))
  {
   Print("[GRZ InstaDrop] Inactive: remove LUGU InstaDropFix from the active mod list before testing GRZ.");
   return;
  }
  GRZInstaDropTransitions settings;
  string error;
  if (!JsonFileLoader<GRZInstaDropTransitions>.LoadFile("GRZ_InstaDropData/transitions.json", settings, error) || !settings || !settings.Drop || !settings.Recover)
  {
   Print("[GRZ InstaDrop] Inactive: no reviewed native transition list. Export the vanilla animation index first.");
   return;
  }
  // Drop timing is handled by a native graph branch, not clip-speed overrides.
  // The companion stance helper queues stand intent until an actual roll.
  string graph;
  GetGame().ConfigGetText("CfgVehicles SurvivorBase enfAnimSys graphName", graph);
  if (!GetGame().ConfigIsExisting("CfgPatches GRZ_InstaDropGraph") || graph != "GRZ_InstaDropGraph/player_main.agr")
  {
   Print("[GRZ InstaDrop] v0.1.9 inactive: graph addon missing or graph overridden. Selected graph=" + graph);
   return;
  }
  Ready = true;
  Print("[GRZ InstaDrop] v0.1.9 graph selected: GRZ_InstaDropGraph/player_main.agr. Throw-ready drop blend=0.1; prone/on-back direct stand blend=0.16. Gun-lift native roll routes enabled; forced stand requires an actual roll. Verify the native roll-start log.");
 }
};
modded class DayZPlayerTypeUtils
{
 override static void DayZPlayerTypeInitMovement(DayZPlayerType pType)
 {
  super.DayZPlayerTypeInitMovement(pType);
  GRZInstaDrop.Apply();
 }
};
