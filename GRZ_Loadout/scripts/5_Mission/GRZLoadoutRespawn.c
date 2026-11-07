class GRZRespawnCharacterChoice
{
 bool RandomCharacter;
 int Expires;
};

modded class MissionServer
{
 protected ref map<string, ref GRZRespawnCharacterChoice> m_GRZRespawnChoices = new map<string, ref GRZRespawnCharacterChoice>;

 override void OnInit()
 {
  super.OnInit();
  DayZGame.Event_OnRPC.Insert(GRZReceiveRespawnChoice);
 }

 void GRZReceiveRespawnChoice(PlayerIdentity sender, Object target, int rpcType, ParamsReadContext ctx)
 {
  if (!g_Game.IsServer() || rpcType != GRZRespawnChoiceRPC.CHOOSE || !sender || target) return;
  int requestId;
  bool randomCharacter;
  if (!ctx.Read(requestId) || !ctx.Read(randomCharacter) || requestId < 1) return;
  bool allowed = true;
  array<Man> players = new array<Man>;
  g_Game.GetPlayers(players);
  foreach (Man man : players)
  {
   PlayerBase player = PlayerBase.Cast(man);
   if (player && player.GetIdentity() && player.GetIdentity().GetId() == sender.GetId() && player.IsAlive())
    allowed = false;
  }
  for (int i = m_GRZRespawnChoices.Count() - 1; i >= 0; i--)
  {
   string key = m_GRZRespawnChoices.GetKey(i);
   GRZRespawnCharacterChoice existing = m_GRZRespawnChoices.Get(key);
   if (!existing || existing.Expires < g_Game.GetTime()) m_GRZRespawnChoices.Remove(key);
  }
  if (allowed)
  {
   GRZRespawnCharacterChoice choice = new GRZRespawnCharacterChoice;
   choice.RandomCharacter = randomCharacter;
   choice.Expires = g_Game.GetTime() + 60000;
   m_GRZRespawnChoices.Set(sender.GetId(), choice);
  }
  ScriptRPC ack = new ScriptRPC;
  ack.Write(requestId);
  ack.Write(allowed);
  ack.Send(null, GRZRespawnChoiceRPC.ACK, true, sender);
 }

 override void OnMissionFinish()
 {
  DayZGame.Event_OnRPC.Remove(GRZReceiveRespawnChoice);
  m_GRZRespawnChoices.Clear();
  super.OnMissionFinish();
 }

 override PlayerBase CreateCharacter(PlayerIdentity identity, vector pos, ParamsReadContext ctx, string characterName)
 {
  GRZSavedOutfit saved = GRZOutfitStore.Load(identity);
  if (saved)
   characterName = saved.Character;
  if (identity)
  {
   GRZRespawnCharacterChoice choice = m_GRZRespawnChoices.Get(identity.GetId());
   m_GRZRespawnChoices.Remove(identity.GetId());
   if (choice && choice.RandomCharacter && choice.Expires >= g_Game.GetTime())
   {
    array<string> characters = g_Game.ListAvailableCharacters();
    if (characters && characters.Count() > 0) characterName = characters.GetRandomElement();
   }
  }
  return super.CreateCharacter(identity, pos, ctx, characterName);
 }

 override PlayerBase OnClientNewEvent(PlayerIdentity identity, vector pos, ParamsReadContext ctx)
 {
  PlayerBase player = super.OnClientNewEvent(identity, pos, ctx);
  if (player)
   g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GRZApplySavedLoadout, 500, false, player);
  return player;
 }

 void GRZApplySavedLoadout(PlayerBase player)
 {
  if (!player || !player.GetIdentity() || !player.IsAlive())
   return;
  // Restore clothing/cargo space first, then run the existing transactional
  // gun grant. Reconnects use ClientReady and do not receive duplicate gear.
  GRZRemoveStarterItems(player);
  GRZApplySavedClothing(player);
  if (GRZDuelServer.Spawned(player)) return;
  GRZSavedWeapon weapon = GRZWeaponStore.Load(player.GetIdentity());
  if (weapon && !GRZLoadoutServer.Equip(player, weapon.Weapon, weapon.Wrap, false))
   Print("[GRZ Loadout] Saved weapon could not be restored; spawn gear preserved.");
 }

 void GRZRemoveStarterItems(PlayerBase player)
 {
  // Called only for a newly created character, after native starter gear.
  // Clear these before dressing/granting the saved loadout to free cargo.
  array<EntityAI> items = new array<EntityAI>;
  player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
  for (int i = items.Count() - 1; i >= 0; i--)
  {
   EntityAI item = items[i];
   if (!item || item == player)
    continue;
   if (item.IsKindOf("SodaCan_ColorBase") || item.IsKindOf("Chemlight_ColorBase") || item.IsKindOf("Pear") || item.IsKindOf("Apple") || item.IsKindOf("Plum"))
    g_Game.ObjectDelete(item);
  }
 }

 void GRZApplySavedClothing(PlayerBase player)
 {
  if (!player || !player.GetIdentity() || !player.IsAlive())
   return;
  GRZSavedOutfit saved = GRZOutfitStore.Load(player.GetIdentity());
  if (!saved)
   return;
  GRZLoadoutServer.Init();
  // A lapsed VIP must not prevent their remaining saved outfit from restoring.
  if (!GRZVipLoadoutPolicy.Allowed(player.GetIdentity()))
  {
   for (int paidIndex = 0; paidIndex < saved.Choices.Count(); paidIndex++)
    if (GRZVipLoadoutPolicy.PaidClothing(saved.Choices[paidIndex])) saved.Choices[paidIndex] = "";
  }
  // Missing/unloaded clothing mods never cause an inventory clear on respawn.
  if (!GRZLoadoutClothingServer.Equip(player, saved.Choices))
   Print("[GRZ Clothing] Saved outfit could not be equipped; spawn gear preserved.");
 }
};
