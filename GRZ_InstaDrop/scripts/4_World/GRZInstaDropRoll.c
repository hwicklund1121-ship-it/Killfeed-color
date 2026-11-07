// Let the existing prone lean/movement controls request a native roll from
// the on-back gun-lift pose. Do not synthesize keys or stand automatically.
class GRZInstaDropRollPolicy
{
 static bool AllowNewRequest(bool enabled, bool rifle, bool callbackActive, bool graphRolling)
 {
  return !enabled || !rifle || (!callbackActive && !graphRolling);
 }
 static bool AllowOnBack(bool enabled, bool living, bool firearm, bool moveCommand, bool prone, bool onBack, bool prepared, bool clear, bool stamina, bool emote, bool charging, bool releasing)
 {
  return enabled && living && firearm && moveCommand && prone && onBack && prepared && clear && stamina && !emote && !charging && !releasing;
 }
};

modded class PlayerBase
{
 protected int m_GRZNextRollTrace;
 protected int m_GRZActiveRollUntil = -1;

 override bool CanRoll()
 {
  HumanCommandMove activeMovement = GetCommand_Move();
  bool rollRifle = GetHumanInventory() && Rifle_Base.Cast(GetHumanInventory().GetEntityInHands()) != null;
  bool callbackActive = m_GRZActiveRollUntil >= 0 && GetGame().GetTime() < m_GRZActiveRollUntil;
  bool graphRolling = activeMovement && activeMovement.IsInRoll();
  if (!GRZInstaDropRollPolicy.AllowNewRequest(GRZInstaDrop.IsEnabled(), rollRifle, callbackActive, graphRolling))
  {
   int traceNow = GetGame().GetTime();
   if (traceNow >= m_GRZNextRollTrace)
   {
    m_GRZNextRollTrace = traceNow + 500;
    Print("[GRZ InstaDrop] v0.3.2 roll continuity: held repeated roll request; callbackActive=" + callbackActive.ToString() + " graphRolling=" + graphRolling.ToString());
   }
   return false;
  }
  if (super.CanRoll()) return true;
  HumanCommandMove movement = GetCommand_Move();
  HumanCommandWeapons weapons = GetCommandModifier_Weapons();
  DayZPlayerImplementThrowing throwing = GetThrowing();
  UAInterface input = GetInputInterface();
  if (!movement || !weapons || !throwing || !input || !GetHumanInventory() || !GetActionManager() || !GetEmoteManager() || !GetWeaponManager()) return false;

  bool living = IsAlive() && !IsUnconscious() && !IsRestrained();
  bool firearm = Rifle_Base.Cast(GetHumanInventory().GetEntityInHands()) != null;
  bool moveCommand = GetCurrentCommandID() == DayZPlayerConstants.COMMANDID_MOVE;
  bool prone = IsPlayerInStance(DayZPlayerConstants.STANCEMASK_PRONE | DayZPlayerConstants.STANCEMASK_RAISEDPRONE);
  bool onBack = movement.IsOnBack();
  bool prepared = throwing.IsThrowingModeEnabled() || weapons.IsThrowingMode();
  bool clear = !GetWeaponManager().IsRunning() && input.SyncedValue_ID(UADropitem) <= 0;
  ActionBase action = GetActionManager().GetRunningAction();
  if (action && !action.CanBePerformedWhileChangingStance()) clear = false;
  bool stamina = CanConsumeStamina(EStaminaConsumers.ROLL);
  bool emote = IsInFBEmoteState() || m_EmoteManager.m_MenuEmote || GetEmoteManager().IsEmotePlaying();
  bool charging = throwing.IsThrowingInProgress();
  bool releasing = throwing.IsThrowingAnimationPlaying();
  bool allowed = GRZInstaDropRollPolicy.AllowOnBack(GRZInstaDrop.IsEnabled(), living, firearm, moveCommand, prone, onBack, prepared, clear, stamina, emote, charging, releasing);

  int now = GetGame().GetTime();
  if (GRZInstaDrop.IsEnabled() && firearm && onBack && prepared && now >= m_GRZNextRollTrace)
  {
   m_GRZNextRollTrace = now + 500;
   Print("[GRZ InstaDrop] v0.3.2 on-back roll: native=false exception=" + allowed.ToString() + " living=" + living.ToString() + " move=" + moveCommand.ToString() + " prone=" + prone.ToString() + " clear=" + clear.ToString() + " stamina=" + stamina.ToString() + " emote=" + emote.ToString() + " charging=" + charging.ToString() + " releasing=" + releasing.ToString());
  }
  return allowed;
 }

 override void OnRollStart(bool isToTheRight)
 {
  super.OnRollStart(isToTheRight);
  if (GRZInstaDrop.IsEnabled() && GetHumanInventory() && Rifle_Base.Cast(GetHumanInventory().GetEntityInHands()) != null) m_GRZActiveRollUntil = GetGame().GetTime() + 1500;
  if (GRZInstaDrop.IsEnabled()) Print("[GRZ InstaDrop] v0.3.2 native roll started; right=" + isToTheRight.ToString());
 }

 override void OnRollFinish()
 {
  super.OnRollFinish();
  m_GRZActiveRollUntil = -1;
  if (GRZInstaDrop.IsEnabled()) Print("[GRZ InstaDrop] v0.3.2 native roll finished.");
 }
};
