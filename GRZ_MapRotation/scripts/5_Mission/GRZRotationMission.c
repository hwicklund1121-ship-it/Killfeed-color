modded class MissionServer
{
 override void OnMissionStart()
 {
  super.OnMissionStart();
  // Editor Loader finishes its synchronous DZE loading in the super chain.
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GRZRotationServer.Init, 3000, false);
 }

 override PlayerBase CreateCharacter(PlayerIdentity identity, vector pos, ParamsReadContext ctx, string characterName)
 {
  if (GRZRotationServer.Enabled)
  {
   GRZRotationArena arena = GRZRotationServer.Settings.Arenas[GRZRotationServer.ActiveIndex];
   if (arena.SafeSpawns.Count() > 0)
    pos = arena.SafeSpawns.GetRandomElement();
  }
  return super.CreateCharacter(identity, pos, ctx, characterName);
 }

 override void OnClientReadyEvent(PlayerIdentity identity, PlayerBase player)
 {
  super.OnClientReadyEvent(identity, player);
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GRZRotationServer.PlayerReady, 1500, false, player);
 }

 override void OnMissionFinish()
 {
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(GRZRotationServer.Init);
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(GRZRotationServer.PlayerReady);
  GRZRotationServer.Stop();
  super.OnMissionFinish();
 }
};
