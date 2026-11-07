// GRZ Movement is the sole owner of the optional fast-prone animation rates.
// InstaDrop only handles stance permissions. No graphs or game animations are
// redistributed. Recovery/roll animations are deliberately not retimed here.
class GRZMovementDropSettings
{
 bool TuneDropInMovement = false;
 float DropSpeed = 4.0;
 ref array<string> Drop = new array<string>;
};
class GRZMovementDropBoost
{
 protected static bool Applied;
 static void Apply()
 {
  if (Applied) return;
  Applied = true;
  if (!GetGame().ConfigIsExisting("CfgPatches GRZ_InstaDropData")) return;
  if (GetGame().ConfigIsExisting("CfgPatches LUGUINSTADROPFIX")) return;
  GRZMovementDropSettings settings;
  string error;
  if (!JsonFileLoader<GRZMovementDropSettings>.LoadFile("GRZ_InstaDropData/transitions.json", settings, error) || !settings || !settings.Drop || !settings.TuneDropInMovement) return;
  float speed = Math.Clamp(settings.DropSpeed, 1.0, 4.0);
  int accepted;
  foreach (string configured : settings.Drop)
  {
   string clip = configured;
   clip.ToLower();
   // Exact category/direction only: never retime gait, throw release, roll or
   // prone-to-standing recovery clips, even if a data file is misconfigured.
   if (clip.IndexOf("dz/anims/anm/player/moves/rifles/transitions/") != 0) continue;
   if (clip.IndexOf("_trans_pne_") < 0 && clip.IndexOf("_transmove_pne_") < 0) continue;
   if (clip.Length() < 4 || clip.Substring(clip.Length() - 4, 4) != ".anm") continue;
   bool ok = DayZPlayerUtils.DebugOverrideAnimationSpeed(clip, speed);
   if (ok) accepted++;
   Print("[GRZ Movement] fast-drop clip=" + clip + " requested=" + speed.ToString() + " accepted=" + ok.ToString());
  }
  Print("[GRZ Movement] Fast-drop-only tuning: accepted " + accepted.ToString() + "/" + settings.Drop.Count().ToString() + " clips, request=" + speed.ToString() + ". Recovery and roll timing unchanged; verify ground contact in game.");
 }
};
