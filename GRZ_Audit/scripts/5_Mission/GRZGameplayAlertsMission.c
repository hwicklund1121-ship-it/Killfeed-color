modded class MissionServer
{
 override void OnMissionStart()
 {
  super.OnMissionStart();
  GRZGameplayAlerts.Init();
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GRZGameplayAlerts.Tick, 1000, true);
 }
 override void OnMissionFinish()
 {
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(GRZGameplayAlerts.Tick);
  super.OnMissionFinish();
 }
};
