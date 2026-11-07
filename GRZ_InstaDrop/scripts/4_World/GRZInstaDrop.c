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
 protected static bool NativePath(string path)
 {
  string lower = path;
  lower.ToLower();
  return lower.IndexOf("dz/anims/anm/player/moves/") == 0 && lower.Length() > 4 && lower.Substring(lower.Length() - 4, 4) == ".anm";
 }
 protected static int ApplyClips(array<string> clips, float speed)
 {
  int accepted;
  foreach (string clip : clips)
  {
   if (!NativePath(clip))
   {
    Print("[GRZ InstaDrop] Rejected non-native clip path: " + clip);
    continue;
   }
   bool ok = DayZPlayerUtils.DebugOverrideAnimationSpeed(clip, speed);
   if (ok) accepted++;
   Print("[GRZ InstaDrop] clip=" + clip + " requested=" + speed.ToString() + " accepted=" + ok.ToString());
  }
  return accepted;
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
  if (!settings.OverrideTransitionSpeeds)
  {
   Ready = true;
   Print("[GRZ InstaDrop] v0.1.3 input-only compatibility mode. Other movement mods retain all animation speeds and graphs.");
   return;
  }
  if (settings.Drop.Count() == 0 || settings.Recover.Count() == 0)
  {
   Print("[GRZ InstaDrop] Inactive: both drop and recovery clip lists are required.");
   return;
  }
  Ready = true;
  // The packaged list is shared by client and server. Native stance, collision,
  // weapon input, stamina, and multiplayer movement commands remain in control.
  int drops = ApplyClips(settings.Drop, Math.Clamp(settings.DropSpeed, 1.0, 4.0));
  int recoveries = ApplyClips(settings.Recover, Math.Clamp(settings.RecoverySpeed, 1.0, 4.0));
  Print("[GRZ InstaDrop] v0.1.3 prototype: accepted " + drops.ToString() + "/" + settings.Drop.Count().ToString() + " drop and " + recoveries.ToString() + "/" + settings.Recover.Count().ToString() + " recovery clips. In-game verification required.");
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
