// Standalone GRZ input integration. Cancel firearm throw preparation on a real
// stance input before asking the native stance handler to perform the change.
// Actual release remains native; no forced stance or duplicated stance rules.
class GRZInstaDropThrowPolicy
{
 static bool ShouldCancel(bool stanceInput, bool firearm, bool movement, bool throwMode, bool charging, bool nativeMode, bool releasing)
 {
  return stanceInput && firearm && movement && !releasing && (throwMode || charging || nativeMode);
 }
};
modded class PlayerBase
{
 protected void GRZCancelThrowPreparationForStance()
 {
  if (!GRZInstaDrop.IsEnabled() || !IsAlive() || IsUnconscious()) return;
  HumanInputController controls = GetInputController();
  HumanCommandWeapons weapons = GetCommandModifier_Weapons();
  DayZPlayerImplementThrowing throwing = GetThrowing();
  if (!controls || !weapons || !throwing || !GetHumanInventory()) return;
  bool firearm = Weapon_Base.Cast(GetHumanInventory().GetEntityInHands()) != null;
  bool movement = GetCommand_Move() != null;
  if (!GRZInstaDropThrowPolicy.ShouldCancel(controls.IsStanceChange(), firearm, movement, throwing.IsThrowingModeEnabled(), throwing.IsThrowingInProgress(), weapons.IsThrowingMode(), throwing.IsThrowingAnimationPlaying())) return;
  // Native cancellation clears charge/progress, weapon throw mode and the
  // controller's throw toggle together. The item stays in the player's hands.
  throwing.CancelThrowing(controls, weapons);
 }
 override bool ModCommandHandlerBefore(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
 {
  GRZCancelThrowPreparationForStance();
  return super.ModCommandHandlerBefore(pDt, pCurrentCommandID, pCurrentCommandFinished);
 }
 override bool CanChangeStance(int previousStance, int newStance)
 {
  // Also cover a native stance query that arrives before this tick's script
  // command handler. Never cancel from passive queries without a stance input.
  if (previousStance != newStance) GRZCancelThrowPreparationForStance();
  return super.CanChangeStance(previousStance, newStance);
 }
};
