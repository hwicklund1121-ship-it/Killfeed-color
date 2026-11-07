class GRZMovementGaitList
{
 float Multiplier = 1.10;
 ref array<string> Animations = new array<string>;
};
class GRZMovementGaitBoost
{
 protected static bool Applied;
 static void Apply()
 {
  if (Applied) return;
  Applied = true;
  GRZMovementGaitList settings;
  string error;
  if (!JsonFileLoader<GRZMovementGaitList>.LoadFile("GRZ_MovementSpeedData/gaits.json", settings, error) || !settings || !settings.Animations)
  {
   Print("[GRZ Movement] No generated native gait list; speed boost inactive. Run the combined installer.");
   return;
  }
  float multiplier = Math.Clamp(settings.Multiplier, 1.0, 1.20);
  int accepted;
  foreach (string clip : settings.Animations)
  {
   // Native API used by the game's own movement-initialization example.
   // Apply on both sides; no position/velocity edits or input-speed overrides.
   if (DayZPlayerUtils.DebugOverrideAnimationSpeed(clip, multiplier)) accepted++;
  }
  Print("[GRZ Movement] Requested gait speed " + multiplier.ToString() + "; engine accepted " + accepted.ToString() + "/" + settings.Animations.Count().ToString() + " clips. Verify distance/time in game.");
 }
};
