// Only the four reviewed rifle-to-standing clips are retimed. The graph keeps
// their native leg/arm motion and exits by remaining animation time.
class GRZInstaDropRecovery
{
 protected static bool Applied;
 static void Apply(bool enabled, float requestedSpeed)
 {
  if (Applied) return;
  if (!enabled)
  {
   Print("[GRZ InstaDrop] v0.3.0 native get-up speed tuning disabled by data.");
   return;
  }
  Applied = true;
  float speed = Math.Clamp(requestedSpeed, 1.0, 3.0);
  array<string> clips = {
   "dz/anims/anm/player/moves/rifles/transitions/p_rfl_pne_trans_erc_low.anm",
   "dz/anims/anm/player/moves/rifles/transitions/p_rfl_pne_trans_erc_ras.anm",
   "dz/anims/anm/player/moves/rifles/transitions/p_rfl_pne_onback_trans_erc_low.anm",
   "dz/anims/anm/player/moves/rifles/transitions/p_rfl_pne_onback_trans_erc_ras.anm"
  };
  int accepted;
  foreach (string clip : clips)
  {
   bool ok = DayZPlayerUtils.DebugOverrideAnimationSpeed(clip, speed);
   if (ok) accepted++;
   Print("[GRZ InstaDrop] v0.3.0 get-up clip=" + clip + " request=" + speed.ToString() + " accepted=" + ok.ToString());
  }
  Print("[GRZ InstaDrop] v0.3.0 native get-up speed: accepted " + accepted.ToString() + "/4; request=" + speed.ToString() + ". Verify actual playback in game.");
 }
};
