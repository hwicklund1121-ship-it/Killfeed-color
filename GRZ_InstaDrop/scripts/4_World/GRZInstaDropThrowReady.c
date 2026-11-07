// Independent GRZ input integration. Native actions include direct stance keys
// as well as the controller's combined flag. No new bindings are registered.
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
 static bool ShouldCancel(bool stanceInput, bool firearm, bool movement, bool throwMode, bool charging, bool nativeMode, bool releasing)
 {
  return stanceInput && firearm && movement && !releasing && (throwMode || charging || nativeMode);
 }
};
modded class PlayerBase
{
 protected int m_GRZStanceInputUntil = -1;
 protected int m_GRZNextInputTrace;
 protected int m_GRZNextStanceTrace;
 protected bool GRZRequestedStance(HumanInputController controls, out bool direct)
 {
  UAInterface input = GetInputInterface();
  bool cycle;
  bool stand;
  bool crouch;
  bool prone;
  if (input)
  {
   cycle = input.SyncedPress("UAStance");
   stand = input.SyncedPress("UAStand");
   crouch = input.SyncedPress("UACrouch");
   prone = input.SyncedPress("UAProne");
  }
  direct = cycle || stand || crouch || prone;
  int now = GetGame().GetTime();
  if (GRZInstaDropThrowPolicy.HasStanceInput(controls.IsStanceChange(), cycle, stand, crouch, prone)) m_GRZStanceInputUntil = now + 250;
  // Preserve the user's actual input across the before-handler/stance-query
  // callback boundary. This is not a forced or replayed movement command.
  return GRZInstaDropThrowPolicy.RecentInput(now, m_GRZStanceInputUntil);
 }
 protected bool GRZCancelThrowPreparationForStance(string context)
 {
  if (!GRZInstaDrop.IsEnabled() || !IsAlive() || IsUnconscious()) return false;
  HumanInputController controls = GetInputController();
  HumanCommandWeapons weapons = GetCommandModifier_Weapons();
  DayZPlayerImplementThrowing throwing = GetThrowing();
  if (!controls || !weapons || !throwing || !GetHumanInventory()) return false;
  bool direct;
  bool controller = controls.IsStanceChange();
  bool requested = GRZRequestedStance(controls, direct);
  bool firearm = Weapon_Base.Cast(GetHumanInventory().GetEntityInHands()) != null;
  bool movement = GetCommand_Move() != null;
  bool mode = throwing.IsThrowingModeEnabled();
  bool charging = throwing.IsThrowingInProgress();
  bool nativeMode = weapons.IsThrowingMode();
  bool releasing = throwing.IsThrowingAnimationPlaying();
  int now = GetGame().GetTime();
  string side = "client";
  if (GetGame().IsServer()) side = "server";
  if (requested && now >= m_GRZNextInputTrace)
  {
   m_GRZNextInputTrace = now + 300;
   Print("[GRZ InstaDrop] v0.1.3 input side=" + side + " hook=" + context + " controller=" + controller.ToString() + " direct=" + direct.ToString() + " firearm=" + firearm.ToString() + " move=" + movement.ToString() + " mode=" + mode.ToString() + " charge=" + charging.ToString() + " release=" + releasing.ToString());
  }
  if (!GRZInstaDropThrowPolicy.ShouldCancel(requested, firearm, movement, mode, charging, nativeMode, releasing)) return false;
  throwing.CancelThrowing(controls, weapons);
  // Log successful cancellation once, without names, IDs or physical keys.
  Print("[GRZ InstaDrop] v0.1.3 cancelled firearm throw preparation side=" + side + " hook=" + context);
  return true;
 }
 override bool ModCommandHandlerBefore(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
 {
  GRZCancelThrowPreparationForStance("before");
  return super.ModCommandHandlerBefore(pDt, pCurrentCommandID, pCurrentCommandFinished);
 }
 override bool CanChangeStance(int previousStance, int newStance)
 {
  if (previousStance != newStance) GRZCancelThrowPreparationForStance("stance-query");
  bool allowed = super.CanChangeStance(previousStance, newStance);
  int now = GetGame().GetTime();
  if (GRZInstaDrop.IsEnabled() && previousStance != newStance && GRZInstaDropThrowPolicy.RecentInput(now, m_GRZStanceInputUntil) && now >= m_GRZNextStanceTrace)
  {
   m_GRZNextStanceTrace = now + 300;
   string side = "client";
   if (GetGame().IsServer()) side = "server";
   float throwKey;
   UAInterface input = GetInputInterface();
   if (input) throwKey = input.SyncedValue_ID(UAThrowitem);
   Print("[GRZ InstaDrop] v0.1.3 stance side=" + side + " from=" + previousStance.ToString() + " to=" + newStance.ToString() + " nativeAllowed=" + allowed.ToString() + " throwAction=" + throwKey.ToString());
  }
  return allowed;
 }
};
