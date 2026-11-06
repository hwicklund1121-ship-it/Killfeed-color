// Native time constants only: retain collision, gait speeds and stamina policy.
modded class CfgGameplayHandler
{
 override static float GetMovementRotationSpeedJog()
 {
  return Math.Min(super.GetMovementRotationSpeedJog(), 0.09);
 }
 override static float GetMovementRotationSpeedSprint()
 {
  return Math.Min(super.GetMovementRotationSpeedSprint(), 0.085);
 }
};
