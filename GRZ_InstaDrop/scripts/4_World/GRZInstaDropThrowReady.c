// Independent GRZ stance permissions for gun-lift -> prone -> roll -> stand.
// Keep throw-ready mode intact. Actual throwing/release and item dropping stay
// restricted. Physical stance checks remain in the native movement controller.
class GRZInstaDropThrowPolicy
{
 static bool HasStanceInput(bool controller, bool cycle, bool stand, bool crouch, bool prone)
 {
  return controller || cycle || stand || crouch || prone;
 }
 static bool RecentInput(int now, int until)
 {
  return until >= 0 && now <= until;
 }
 static bool ShouldForceStand(bool rolling, bool queued)
 {
  return rolling && queued;
 }
 static bool AllowLegacy(bool requested, bool firearm, bool movement, bool clear, bool prepared, bool rolling, bool proneTarget, bool standingTarget, bool charging, bool releasing)
 {
  return requested && firearm && movement && clear && !charging && !releasing && ((prepared && proneTarget) || (rolling && standingTarget));
 }
};
modded class PlayerBase
{
 protected int m_GRZStanceInputUntil = -1;
 protected int m_GRZStandRequestUntil = -1;
 protected int m_GRZNextStanceTrace;
 protected bool GRZRequestedStance()
 {
  HumanInputController controls = GetInputController();
  UAInterface input = GetInputInterface();
  if (!controls || !input) return false;
  bool cycle = input.SyncedPress("UAStance");
  bool stand = input.SyncedPress("UAStand");
  bool crouch = input.SyncedPress("UACrouch");
  bool prone = input.SyncedPress("UAProne");
  int now = GetGame().GetTime();
  if (GRZInstaDropThrowPolicy.HasStanceInput(controls.IsStanceChange(), cycle, stand, crouch, prone)) m_GRZStanceInputUntil = now + 250;
  return GRZInstaDropThrowPolicy.RecentInput(now, m_GRZStanceInputUntil);
 }
 protected bool GRZLegacyStanceAllowed(int newStance, bool queuedStand = false)
 {
  if (!GRZInstaDrop.IsEnabled() || !IsAlive() || IsUnconscious() || IsRestrained()) return false;
  HumanCommandMove movement = GetCommand_Move();
  HumanCommandWeapons weapons = GetCommandModifier_Weapons();
  DayZPlayerImplementThrowing throwing = GetThrowing();
  UAInterface input = GetInputInterface();
  if (!movement || !weapons || !throwing || !input || !GetHumanInventory() || !GetActionManager() || !GetEmoteManager() || !GetWeaponManager()) return false;
  bool firearm = Weapon_Base.Cast(GetHumanInventory().GetEntityInHands()) != null;
  bool moving = GetCurrentCommandID() == DayZPlayerConstants.COMMANDID_MOVE;
  bool clear = !GetEmoteManager().IsEmotePlaying() && !GetWeaponManager().IsRunning();
  ActionBase action = GetActionManager().GetRunningAction();
  if (action && !action.CanBePerformedWhileChangingStance()) clear = false;
  // Preserve native item-drop protection. Holding the throw-mode key alone is
  // the intentional exception; charge/release remain blocked below.
  if (input.SyncedValue_ID(UADropitem) > 0) clear = false;
  HumanMovementState state = new HumanMovementState;
  GetMovementState(state);
  bool fromProne = state.m_iStanceIdx == DayZPlayerConstants.STANCEIDX_PRONE || state.m_iStanceIdx == DayZPlayerConstants.STANCEIDX_RAISEDPRONE;
  bool proneTarget = newStance == DayZPlayerConstants.STANCEIDX_PRONE || newStance == DayZPlayerConstants.STANCEIDX_RAISEDPRONE;
  bool standingTarget = newStance == DayZPlayerConstants.STANCEIDX_ERECT || newStance == DayZPlayerConstants.STANCEIDX_RAISEDERECT;
  proneTarget = proneTarget && !fromProne;
  standingTarget = standingTarget && fromProne;
  if (proneTarget && GetCurrentWaterLevel() > GetDayZPlayerType().CommandSwimSettingsW().m_fToCrouchLevel) clear = false;
  bool prepared = throwing.IsThrowingModeEnabled() || weapons.IsThrowingMode();
  return GRZInstaDropThrowPolicy.AllowLegacy(GRZRequestedStance() || (queuedStand && standingTarget), firearm, moving, clear, prepared, movement.IsInRoll() || movement.IsOnBack(), proneTarget, standingTarget, throwing.IsThrowingInProgress(), throwing.IsThrowingAnimationPlaying());
 }
 override bool ModCommandHandlerBefore(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
 {
  if (GRZInstaDrop.IsEnabled())
  {
   bool requested = GRZRequestedStance();
   UAInterface input = GetInputInterface();
   int now = GetGame().GetTime();
   // Direct Stand can be observed even if the graph never asks the stance
   // permission callback during a roll. Other controls keep native targeting.
   if (input && (input.SyncedPress("UAProne") || input.SyncedPress("UACrouch"))) m_GRZStandRequestUntil = -1;
   if (requested && input && input.SyncedPress("UAStand")) m_GRZStandRequestUntil = now + 750;
   HumanCommandMove movement = GetCommand_Move();
   if (movement && GRZInstaDropThrowPolicy.ShouldForceStand(movement.IsInRoll(), GRZInstaDropThrowPolicy.RecentInput(now, m_GRZStandRequestUntil)) && GRZLegacyStanceAllowed(DayZPlayerConstants.STANCEIDX_ERECT, true))
   {
    // Ask the native stance test before issuing its stand-up request.
    // No teleport, persistent ForceStance lock, or replacement animation graph.
    if (DayZPlayerUtils.PlayerCanChangeStance(this, DayZPlayerConstants.STANCEIDX_ERECT, true))
    {
     movement.ForceStanceUp(DayZPlayerConstants.STANCEIDX_ERECT);
     Print("[GRZ InstaDrop] v0.3.1 actual-roll stand requested; graph recovery has a minimum 0.12-second roll segment.");
    }
    else Print("[GRZ InstaDrop] v0.3.1 native stance test rejected actual-roll stand.");
    m_GRZStandRequestUntil = -1;
   }
  }
  return super.ModCommandHandlerBefore(pDt, pCurrentCommandID, pCurrentCommandFinished);
 }
 override bool CanChangeStance(int previousStance, int newStance)
 {
  bool allowed = super.CanChangeStance(previousStance, newStance);
  bool legacy;
  int now = GetGame().GetTime();
  if (previousStance != newStance && !allowed)
  {
   legacy = GRZLegacyStanceAllowed(newStance);
   if (legacy)
   {
    allowed = true;
    if (newStance == DayZPlayerConstants.STANCEIDX_ERECT || newStance == DayZPlayerConstants.STANCEIDX_RAISEDERECT) m_GRZStandRequestUntil = now + 750;
   }
  }
  if (GRZInstaDrop.IsEnabled() && previousStance != newStance && GRZInstaDropThrowPolicy.RecentInput(now, m_GRZStanceInputUntil) && now >= m_GRZNextStanceTrace)
  {
   m_GRZNextStanceTrace = now + 300;
   Print("[GRZ InstaDrop] v0.3.1 stance from=" + previousStance.ToString() + " to=" + newStance.ToString() + " allowed=" + allowed.ToString() + " legacyException=" + legacy.ToString() + " gun-lift mode preserved");
  }
  return allowed;
 }
};
