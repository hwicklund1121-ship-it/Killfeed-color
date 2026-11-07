// Independent prototype using the public native DayZ animation API.
// No third-party animation graphs, scripts, assets or forced player positions.
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
  // Only GRZ Movement applies clip rates. This module retains the working
  // v0.1.4 gun-lift/roll stance logic and never overwrites another graph/rate.
  Ready = true;
  Print("[GRZ InstaDrop] v0.1.5 stance permissions active. Fast drop is owned by GRZ Movement; roll-to-stand logic retained.");
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
