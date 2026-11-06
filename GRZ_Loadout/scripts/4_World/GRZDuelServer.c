class GRZDuelChallenge
{
 string Id;
 string A;
 string B;
 string MapId;
 int BestOf;
 int Expires;
 Weapon_Base WeaponA;
};
class GRZDuelMatch
{
 string Id;
 string A;
 string B;
 string SteamA;
 string SteamB;
 string NameA;
 string NameB;
 ref GRZDuelMap Map;
 ref GRZDuelKit KitA;
 ref GRZDuelKit KitB;
 int BestOf;
 int ScoreA;
 int ScoreB;
 int Phase; // 0 waiting for respawn/gear, 1 countdown, 2 fighting
 int Due;
 int LastCount = -1;
 int Draws;
 bool DeadA;
 bool DeadB;
 bool Closing;
 int MissingA = -1;
 int MissingB = -1;
 int OutsideA = -1;
 int OutsideB = -1;
};
class GRZDuelReturn
{
 string Uid;
 vector Position;
};
class GRZDuelHooks
{
 // Map rotation subscribes without making loadout depend on map rotation.
 static ref ScriptInvoker ReturnToArena = new ScriptInvoker;
};
class GRZDuelServer
{
 static ref GRZDuelSettings Settings;
 static ref array<ref GRZDuelChallenge> Challenges = new array<ref GRZDuelChallenge>;
 static ref array<ref GRZDuelMatch> Matches = new array<ref GRZDuelMatch>;
 static ref array<ref GRZDuelReturn> Returns = new array<ref GRZDuelReturn>;
 static int Sequence;
 static int LastBroadcast;
 static string Uid(PlayerBase p)
 {
  if (p && p.GetIdentity()) return p.GetIdentity().GetId();
  return "";
 }
 static PlayerBase Player(string uid)
 {
  array<Man> players = new array<Man>;
  g_Game.GetPlayers(players);
  foreach (Man man : players)
  {
   PlayerBase p = PlayerBase.Cast(man);
   if (Uid(p) == uid && uid != "") return p;
  }
  return null;
 }
 static bool Inside(vector pos, GRZDuelMap arena)
 {
  float dx = pos[0] - arena.Center[0];
  float dz = pos[2] - arena.Center[2];
  return dx * dx + dz * dz <= arena.Radius * arena.Radius;
 }
 static bool ValidMap(GRZDuelMap arena)
 {
  if (!arena || !arena.Enabled || arena.Id == "" || arena.Name == "" || arena.Radius < 10 || arena.Radius > 1000) return false;
  if (arena.SpawnA[0] <= 0 || arena.SpawnA[2] <= 0 || arena.SpawnB[0] <= 0 || arena.SpawnB[2] <= 0) return false;
  if (!Inside(arena.SpawnA, arena) || !Inside(arena.SpawnB, arena) || vector.DistanceSq(arena.SpawnA, arena.SpawnB) < 25) return false;
  if (g_Game.SurfaceIsSea(arena.SpawnA[0], arena.SpawnA[2]) || g_Game.SurfaceIsSea(arena.SpawnB[0], arena.SpawnB[2])) return false;
  return true;
 }
 static GRZDuelMap Arena(string id)
 {
  if (!Settings) return null;
  foreach (GRZDuelMap arena : Settings.Maps)
   if (arena.Id == id && ValidMap(arena)) return arena;
  return null;
 }
 static GRZDuelMatch MatchFor(string uid)
 {
  if (uid == "") return null;
  foreach (GRZDuelMatch match : Matches)
   if (match.A == uid || match.B == uid) return match;
  return null;
 }
 static bool Busy(string id)
 {
  foreach (GRZDuelMatch match : Matches)
   if (match.Map.Id == id) return true;
  return false;
 }
 static GRZDuelChallenge Pending(string uid)
 {
  foreach (GRZDuelChallenge challenge : Challenges)
   if (challenge.A == uid || challenge.B == uid) return challenge;
  return null;
 }
 static bool Ready(PlayerBase p)
 {
  return p && p.GetIdentity() && p.IsAlive() && !p.IsUnconscious() && !p.IsRestrained() && !p.GetCommand_Vehicle();
 }
 static void Init()
 {
  if (Settings || !g_Game.IsServer()) return;
  MakeDirectory("$profile:GRZ_Duels");
  Settings = new GRZDuelSettings;
  string path = "$profile:GRZ_Duels/config.json";
  string error;
  if (FileExist(path))
  {
   if (!JsonFileLoader<GRZDuelSettings>.LoadFile(path, Settings, error) || !Settings || !Settings.Maps)
   {
    Settings = new GRZDuelSettings;
    Settings.Enabled = false;
    Print("[GRZ Duels] Cannot read config; duels disabled. Existing config preserved.");
   }
  }
  else
  {
   GRZDuelMap example = new GRZDuelMap;
   example.Id = "duel_arena_1";
   example.Name = "Duel Arena 1";
   example.Enabled = false;
   Settings.Maps.Insert(example);
   JsonFileLoader<GRZDuelSettings>.SaveFile(path, Settings, error);
  }
  array<string> mapIds = new array<string>;
  foreach (GRZDuelMap configured : Settings.Maps)
  {
   if (!configured || !configured.Enabled) continue;
   if (mapIds.Find(configured.Id) >= 0)
   {
    Settings.Enabled = false;
    Print("[GRZ Duels] Duplicate enabled map ID; duels disabled until config is corrected.");
   }
   mapIds.Insert(configured.Id);
  }
  Settings.ChallengeSeconds = 60;
  Settings.CountdownSeconds = 5;
  Settings.RespawnWaitSeconds = Math.Clamp(Settings.RespawnWaitSeconds, 30, 180);
  Settings.RoundSeconds = Math.Clamp(Settings.RoundSeconds, 30, 600);
  GRZDuelRatingStore.Init();
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, 250, true);
 }
 static void Tell(PlayerBase p, string message)
 {
  if (!p || !p.GetIdentity()) return;
  ScriptRPC rpc = new ScriptRPC;
  rpc.Write(message);
  rpc.Send(p, GRZDuelRPC.NOTICE, true, p.GetIdentity());
 }
 static void Both(GRZDuelMatch match, string message)
 {
  Tell(Player(match.A), message);
  Tell(Player(match.B), message);
 }
 static void Snapshot(PlayerBase p)
 {
  Init();
  if (!p || !p.GetIdentity()) return;
  GRZDuelSnapshot data = new GRZDuelSnapshot;
  data.Elo = GRZDuelRatingStore.Elo(p.GetIdentity().GetPlainId());
  if (!Settings.Enabled) data.Status = "Duels are disabled on this server.";
  else if (!GRZDuelRatingStore.Writable) data.Status = "Ranked duels are temporarily unavailable. Ask a server admin.";
  array<Man> players = new array<Man>;
  g_Game.GetPlayers(players);
  foreach (Man man : players)
  {
   PlayerBase other = PlayerBase.Cast(man);
   if (!other || !other.GetIdentity()) continue;
   GRZDuelPlayerRow row = new GRZDuelPlayerRow;
   row.Id = Uid(other);
   row.Name = other.GetIdentity().GetName();
   row.Elo = GRZDuelRatingStore.Elo(other.GetIdentity().GetPlainId());
   Weapon_Base held = Weapon_Base.Cast(other.GetHumanInventory().GetEntityInHands());
   if (held)
   {
    g_Game.ConfigGetText("CfgWeapons " + held.GetType() + " displayName", row.Weapon);
    if (row.Weapon == "") row.Weapon = held.GetType();
   }
   row.Available = Ready(other) && held && !MatchFor(row.Id) && !Pending(row.Id);
   data.Players.Insert(row);
  }
  foreach (GRZDuelMap arena : Settings.Maps)
  {
   if (!ValidMap(arena)) continue;
   GRZDuelMapRow mapRow = new GRZDuelMapRow;
   mapRow.Id = arena.Id;
   mapRow.Name = arena.Name;
   mapRow.Busy = Busy(arena.Id);
   data.Maps.Insert(mapRow);
  }
  if (data.Maps.Count() == 0 && data.Status == "") data.Status = "Duel maps are not available yet. Ask a server admin.";
  GRZDuelChallenge pending = Pending(Uid(p));
  if (pending)
  {
   data.PendingId = pending.Id;
   data.Incoming = pending.B == Uid(p);
   string otherId = pending.A;
   if (!data.Incoming) otherId = pending.B;
   PlayerBase sender = Player(otherId);
   if (sender) data.PendingName = sender.GetIdentity().GetName();
   GRZDuelMap chosen = Arena(pending.MapId);
   if (chosen) data.PendingMap = chosen.Name;
   if (pending.WeaponA) g_Game.ConfigGetText("CfgWeapons " + pending.WeaponA.GetType() + " displayName", data.PendingGun);
   data.PendingBestOf = pending.BestOf;
   data.PendingSeconds = Math.Max(0, (pending.Expires - g_Game.GetTime() + 999) / 1000);
  }
  GRZDuelMatch match = MatchFor(Uid(p));
  if (match)
  {
   data.InMatch = true;
   data.Phase = match.Phase;
   data.AutoRespawn = true;
   data.BestOf = match.BestOf;
   data.MapName = match.Map.Name;
   data.ScoreSelf = match.ScoreA;
   data.ScoreOpponent = match.ScoreB;
   data.Opponent = match.NameB;
   if (match.B == Uid(p))
   {
    data.ScoreSelf = match.ScoreB;
    data.ScoreOpponent = match.ScoreA;
    data.Opponent = match.NameA;
   }
  }
  foreach (GRZDuelReturn back : Returns)
   if (back.Uid == Uid(p)) data.AutoRespawn = true;
  ScriptRPC rpc = new ScriptRPC;
  rpc.Write(data);
  rpc.Send(p, GRZDuelRPC.SNAPSHOT, true, p.GetIdentity());
 }
 static void SendChallenge(PlayerBase from, string targetId, string mapId, int bestOf)
 {
  Init();
  PlayerBase target = Player(targetId);
  if (!Settings.Enabled || !GRZDuelRatingStore.Writable || !Ready(from) || !Ready(target) || from == target) { Tell(from, "That player is not available for a duel."); return; }
  if (bestOf != 1 && bestOf != 3 && bestOf != 5) return;
  if (!Arena(mapId) || Busy(mapId)) { Tell(from, "That duel map is unavailable."); return; }
  if (Pending(Uid(from)) || Pending(Uid(target)) || MatchFor(Uid(from)) || MatchFor(Uid(target))) { Tell(from, "One of you already has a challenge or duel."); return; }
  if (!GRZDuelKits.Capture(from) || !GRZDuelKits.Capture(target)) { Tell(from, "Both players must hold a supported single-muzzle firearm with valid ammo configuration."); return; }
  GRZDuelChallenge challenge = new GRZDuelChallenge;
  Sequence++;
  challenge.Id = g_Game.GetTime().ToString() + "-" + Sequence.ToString();
  challenge.A = Uid(from);
  challenge.B = Uid(target);
  challenge.MapId = mapId;
  challenge.BestOf = bestOf;
  challenge.WeaponA = Weapon_Base.Cast(from.GetHumanInventory().GetEntityInHands());
  challenge.Expires = g_Game.GetTime() + 60000;
  Challenges.Insert(challenge);
  Tell(target, from.GetIdentity().GetName() + " challenged you to a duel. Accept or decline within 60 seconds.");
  Snapshot(from);
  Snapshot(target);
 }
 static void Answer(PlayerBase p, string id, bool accept)
 {
  GRZDuelChallenge challenge = Pending(Uid(p));
  if (!challenge || challenge.Id != id) return;
  if (!accept || challenge.A == Uid(p))
  {
   Challenges.RemoveItem(challenge);
   Tell(Player(challenge.A), "Duel challenge declined or cancelled.");
   Tell(Player(challenge.B), "Duel challenge declined or cancelled.");
   Snapshot(Player(challenge.A)); Snapshot(Player(challenge.B));
   return;
  }
  PlayerBase a = Player(challenge.A);
  PlayerBase b = Player(challenge.B);
  GRZDuelMap arena = Arena(challenge.MapId);
  if (g_Game.GetTime() >= challenge.Expires || !Ready(a) || !Ready(b) || !arena || Busy(challenge.MapId)) { Tell(p, "The challenge expired or is no longer available."); return; }
  if (a.GetHumanInventory().GetEntityInHands() != challenge.WeaponA) { Tell(p, "The challenger changed guns. Send a fresh challenge."); return; }
  GRZDuelKit kitA = GRZDuelKits.Capture(a);
  GRZDuelKit kitB = GRZDuelKits.Capture(b);
  if (!kitA || !kitB) { Tell(p, "Both players must hold their duel gun."); return; }
  GRZDuelMatch match = new GRZDuelMatch;
  match.Id = challenge.Id;
  match.A = challenge.A; match.B = challenge.B;
  match.SteamA = a.GetIdentity().GetPlainId(); match.SteamB = b.GetIdentity().GetPlainId();
  match.NameA = a.GetIdentity().GetName(); match.NameB = b.GetIdentity().GetName();
  match.KitA = kitA; match.KitB = kitB; match.Map = arena; match.BestOf = challenge.BestOf;
  match.Due = g_Game.GetTime() + Settings.RespawnWaitSeconds * 1000;
  for (int r = Returns.Count() - 1; r >= 0; r--)
   if (Returns[r].Uid == match.A || Returns[r].Uid == match.B) Returns.Remove(r);
  GRZDuelReturn returnA = new GRZDuelReturn;
  returnA.Uid = match.A; returnA.Position = a.GetPosition(); Returns.Insert(returnA);
  GRZDuelReturn returnB = new GRZDuelReturn;
  returnB.Uid = match.B; returnB.Position = b.GetPosition(); Returns.Insert(returnB);
  Matches.Insert(match);
  Challenges.RemoveItem(challenge);
  a.m_GRZDuelMatch = match.Id; b.m_GRZDuelMatch = match.Id;
  Both(match, "Duel accepted. Lower your weapon while the round is prepared.");
  Snapshot(a); Snapshot(b);
 }
 static void Heal(PlayerBase p)
 {
  DamageSystem.ResetAllZones(p);
  p.SetHealthMax("", "Health");
  p.SetHealthMax("", "Blood");
  p.SetHealthMax("", "Shock");
  if (p.GetBleedingManagerServer()) p.GetBleedingManagerServer().RemoveAllSources();
 }
 static void Side(PlayerBase p, GRZDuelMatch match, bool sideA)
 {
  if (!p || !p.IsAlive()) return;
  vector position = match.Map.SpawnB;
  float facing = match.Map.FacingB;
  if (sideA) { position = match.Map.SpawnA; facing = match.Map.FacingA; }
  // Configured DZE arenas use natural ground. Preserve explicit Y for future roof maps.
  if (match.Map.SurfaceSpawns)
  {
   float terrain = g_Game.SurfaceY(position[0], position[2]);
   float roadway = g_Game.SurfaceRoadY3D(position[0], terrain + 50, position[2], RoadSurfaceDetection.UNDER);
   position[1] = Math.Max(terrain, roadway) + 0.2;
  }
  else if (match.Map.GroundSpawns)
   position[1] = g_Game.SurfaceY(position[0], position[2]) + 0.2;
  p.SetPosition(position);
  p.SetOrientation(Vector(facing, 0, 0));
 }
 static void Return(PlayerBase p)
 {
  if (!p || !p.IsAlive()) return;
  for (int i = Returns.Count() - 1; i >= 0; i--)
  {
   GRZDuelReturn back = Returns[i];
   if (back.Uid != Uid(p)) continue;
   p.m_GRZDuelMatch = "";
   Heal(p);
   p.SetPosition(back.Position);
   Returns.Remove(i);
   GRZDuelHooks.ReturnToArena.Invoke(p);
   Snapshot(p);
   return;
  }
 }
 static void Finish(GRZDuelMatch match, int winner, string reason)
 {
  if (Matches.Find(match) < 0) return;
  Matches.RemoveItem(match);
  int delta;
  string result = reason;
  if (winner >= 0)
  {
   string winningId = match.SteamA;
   string losingId = match.SteamB;
   string winningName = match.NameA;
   if (winner == 1) { winningId = match.SteamB; losingId = match.SteamA; winningName = match.NameB; }
   result = winningName + " wins the duel! " + match.ScoreA.ToString() + " - " + match.ScoreB.ToString();
   if (reason != "") result += " | " + reason;
   if (GRZDuelRatingStore.Result(winningId, losingId, delta)) result += " | Elo +" + delta.ToString() + " / -" + delta.ToString();
   else result += " | Rating could not be saved; match not rated.";
  }
  PlayerBase a = Player(match.A);
  PlayerBase b = Player(match.B);
  if (a) { a.m_GRZDuelMatch = ""; Tell(a, result); Return(a); Snapshot(a); }
  if (b) { b.m_GRZDuelMatch = ""; Tell(b, result); Return(b); Snapshot(b); }
 }
 static void Died(string uid)
 {
  GRZDuelMatch match = MatchFor(uid);
  if (!match || match.Phase != 2) return;
  if (uid == match.A) match.DeadA = true;
  if (uid == match.B) match.DeadB = true;
  if (!match.Closing)
  {
   match.Closing = true;
   g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(ResolveRound, 100, false, match.Id);
  }
 }
 static void ResolveRound(string id)
 {
  GRZDuelMatch match;
  foreach (GRZDuelMatch entry : Matches) if (entry.Id == id) { match = entry; break; }
  if (!match || match.Phase != 2) return;
  if (match.DeadA && match.DeadB) match.Draws++;
  else if (match.DeadA) match.ScoreB++;
  else if (match.DeadB) match.ScoreA++;
  else match.Draws++;
  int firstTo = (match.BestOf + 1) / 2;
  if (match.ScoreA >= firstTo) { Finish(match, 0, ""); return; }
  if (match.ScoreB >= firstTo) { Finish(match, 1, ""); return; }
  if (match.Draws >= 3) { Finish(match, -1, "Duel ended in a draw. Elo unchanged."); return; }
  match.Phase = 0;
  match.Closing = false;
  match.DeadA = false; match.DeadB = false;
  match.Due = g_Game.GetTime() + Settings.RespawnWaitSeconds * 1000;
  Side(Player(match.A), match, true); Side(Player(match.B), match, false);
  Both(match, "Round over. " + match.ScoreA.ToString() + " - " + match.ScoreB.ToString() + " | Next round is being prepared.");
  Snapshot(Player(match.A)); Snapshot(Player(match.B));
 }
 static void Connected(PlayerBase p)
 {
  Init();
  if (!p || !p.IsAlive()) return;
  GRZDuelRatingStore.Touch(p);
  GRZDuelMatch match = MatchFor(Uid(p));
  if (match) p.m_GRZDuelMatch = match.Id;
  else Return(p);
  Snapshot(p);
 }
 static bool Spawned(PlayerBase p)
 {
  Init();
  GRZDuelMatch match = MatchFor(Uid(p));
  if (match)
  {
   p.m_GRZDuelMatch = match.Id;
   Side(p, match, Uid(p) == match.A);
   Snapshot(p);
   return true;
  }
  Return(p);
  return false;
 }
 static bool AllowDamage(PlayerBase target, EntityAI source)
 {
  if (!g_Game.IsServer()) return true;
  GRZDuelMatch match = MatchFor(Uid(target));
  PlayerBase attacker = PlayerBase.Cast(source);
  if (!attacker && source) attacker = PlayerBase.Cast(source.GetHierarchyRootPlayer());
  GRZDuelMatch attacking = MatchFor(Uid(attacker));
  if (!match && !attacking) return true;
  if (!match || !attacking || match != attacking || match.Phase != 2 || target == attacker) return false;
  Weapon_Base held = Weapon_Base.Cast(attacker.GetHumanInventory().GetEntityInHands());
  GRZDuelKit permitted = match.KitB;
  if (Uid(attacker) == match.A) permitted = match.KitA;
  if (!held || held.GetType() != permitted.Gun.Type) return false;
  return Inside(target.GetPosition(), match.Map) && Inside(attacker.GetPosition(), match.Map);
 }
 static void Leave(PlayerBase p)
 {
  GRZDuelMatch match = MatchFor(Uid(p));
  if (!match) return;
  int winner = 0;
  if (match.A == Uid(p)) winner = 1;
  Finish(match, winner, "Forfeit");
 }
 static void Tick()
 {
  GRZDuelRatingStore.Flush();
  int now = g_Game.GetTime();
  for (int c = Challenges.Count() - 1; c >= 0; c--)
  {
   GRZDuelChallenge challenge = Challenges[c];
   PlayerBase challengeA = Player(challenge.A);
   PlayerBase challengeB = Player(challenge.B);
   if (challenge.Expires > now && challengeA && challengeB && challengeA.IsAlive() && challengeB.IsAlive()) continue;
   Challenges.Remove(c);
   Tell(Player(challenge.A), "Duel challenge expired."); Tell(Player(challenge.B), "Duel challenge expired.");
   Snapshot(Player(challenge.A)); Snapshot(Player(challenge.B));
  }
  for (int i = Matches.Count() - 1; i >= 0; i--)
  {
   GRZDuelMatch match = Matches[i];
   PlayerBase a = Player(match.A);
   PlayerBase b = Player(match.B);
   if (!a || !b)
   {
    if (!a && match.MissingA < 0) match.MissingA = now;
    if (!b && match.MissingB < 0) match.MissingB = now;
    bool expiredA = !a && now - match.MissingA >= 10000;
    bool expiredB = !b && now - match.MissingB >= 10000;
    // Replacing a dead network entity during respawn is not a disconnect.
    if (match.Phase == 0) { expiredA = !a && now >= match.Due; expiredB = !b && now >= match.Due; }
    if (!a && !b && (expiredA || expiredB)) Finish(match, -1, "Duel cancelled: both players disconnected.");
    else if (expiredA && b) Finish(match, 1, "Disconnect/respawn forfeit");
    else if (expiredB && a) Finish(match, 0, "Disconnect/respawn forfeit");
    continue;
   }
   match.MissingA = -1; match.MissingB = -1;
   a.m_GRZDuelMatch = match.Id; b.m_GRZDuelMatch = match.Id;
   if (match.Phase == 0)
   {
    if (now >= match.Due)
    {
     int timeoutWinner = -1;
     if (Ready(a) && !a.IsRaised() && (!Ready(b) || b.IsRaised())) timeoutWinner = 0;
     if (Ready(b) && !b.IsRaised() && (!Ready(a) || a.IsRaised())) timeoutWinner = 1;
     Finish(match, timeoutWinner, "Respawn/gear preparation timeout"); continue;
    }
    if (!a.IsAlive() || !b.IsAlive()) continue;
    Heal(a); Heal(b);
    if (!Ready(a) || !Ready(b) || a.IsRaised() || b.IsRaised()) continue;
    if (!GRZLoadoutServer.Equip(a, match.KitA.Gun.Type, "", false, match.KitA) || !GRZLoadoutServer.Equip(b, match.KitB.Gun.Type, "", false, match.KitB))
    { Finish(match, -1, "Duel cancelled: weapon preparation failed. Elo unchanged."); continue; }
    Side(a, match, true); Side(b, match, false);
    match.Phase = 1; match.LastCount = -1; match.Due = now + 5000;
   }
   if (match.Phase == 1)
   {
    Side(a, match, true); Side(b, match, false);
    int left = Math.Max(0, (match.Due - now + 999) / 1000);
    if (left != match.LastCount)
    {
     match.LastCount = left;
     if (left > 0) Both(match, "DUEL STARTS IN " + left.ToString());
     else
     {
      match.Phase = 2; match.Due = now + Settings.RoundSeconds * 1000;
      Both(match, "GO!"); Snapshot(a); Snapshot(b);
     }
    }
   }
   else if (match.Phase == 2 && !match.Closing)
   {
    if (!a.IsAlive()) Died(match.A);
    if (!b.IsAlive()) Died(match.B);
    if (now >= match.Due) { match.Closing = true; ResolveRound(match.Id); continue; }
    if (!Inside(a.GetPosition(), match.Map))
    {
     if (match.OutsideA < 0) { match.OutsideA = now; Tell(a, "Outside duel arena: returning to your side in 5 seconds."); }
     if (now - match.OutsideA >= 5000) { Side(a, match, true); match.OutsideA = -1; }
    }
    else match.OutsideA = -1;
    if (!Inside(b.GetPosition(), match.Map))
    {
     if (match.OutsideB < 0) { match.OutsideB = now; Tell(b, "Outside duel arena: returning to your side in 5 seconds."); }
     if (now - match.OutsideB >= 5000) { Side(b, match, false); match.OutsideB = -1; }
    }
    else match.OutsideB = -1;
   }
  }
  if (now - LastBroadcast > 2500)
  {
   LastBroadcast = now;
   foreach (GRZDuelMatch live : Matches) { Snapshot(Player(live.A)); Snapshot(Player(live.B)); }
  }
 }
 static void Stop()
 {
  GRZDuelRatingStore.Flush(true);
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(Tick);
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(ResolveRound);
  Challenges.Clear(); Matches.Clear(); Returns.Clear();
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(Connected);
  Settings = null;
 }
};
modded class PlayerBase
{
 string m_GRZDuelMatch;
 protected bool m_GRZDuelDeathRecorded;
 protected int m_GRZDuelRequestAt = -10000;
 protected int m_GRZDuelActionAt = -10000;
 override void OnConnect()
 {
  super.OnConnect();
  if (g_Game.IsServer()) g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(GRZDuelServer.Connected, 1500, false, this);
 }
 override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
 {
  if (!GRZDuelServer.AllowDamage(this, source)) return false;
  return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
 }
 override void EEKilled(Object killer)
 {
  string uid = GRZDuelServer.Uid(this);
  if (g_Game.IsServer() && !m_GRZDuelDeathRecorded)
  {
   m_GRZDuelDeathRecorded = true;
   PlayerBase attacker = PlayerBase.Cast(killer);
   EntityAI cause = EntityAI.Cast(killer);
   if (!attacker && cause) attacker = PlayerBase.Cast(cause.GetHierarchyRootPlayer());
   // Siphon runs once per confirmed survivor death, independently of stats storage.
   // Never revive a dead killer or reward a suicide.
   if (attacker && attacker != this && attacker.IsAlive() && attacker.GetIdentity())
    GRZDuelServer.Heal(attacker);
   GRZDuelRatingStore.Death(this,attacker);
  }
  super.EEKilled(killer);
  if (g_Game.IsServer()) GRZDuelServer.Died(uid);
 }
 override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
 {
  if ((rpc_type >= GRZDuelRPC.REQUEST && rpc_type <= GRZDuelRPC.ANSWER) || rpc_type == GRZDuelRPC.LEAVE)
  {
   if (!g_Game.IsServer() || !sender || !GetIdentity() || sender.GetId() != GetIdentity().GetId()) return;
   int now = g_Game.GetTime();
   if (rpc_type == GRZDuelRPC.REQUEST)
   {
    if (now - m_GRZDuelRequestAt < 1000) return;
    m_GRZDuelRequestAt = now;
    GRZDuelServer.Snapshot(this);
    return;
   }
   if (now - m_GRZDuelActionAt < 500) return;
   m_GRZDuelActionAt = now;
   if (rpc_type == GRZDuelRPC.CHALLENGE)
   {
    string target; string arena; int bestOf;
    if (ctx.Read(target) && ctx.Read(arena) && ctx.Read(bestOf)) GRZDuelServer.SendChallenge(this, target, arena, bestOf);
   }
   else if (rpc_type == GRZDuelRPC.ANSWER)
   {
    string challenge; bool accept;
    if (ctx.Read(challenge) && ctx.Read(accept)) GRZDuelServer.Answer(this, challenge, accept);
   }
   else if (rpc_type == GRZDuelRPC.LEAVE) GRZDuelServer.Leave(this);
   return;
  }
  if (rpc_type == GRZDuelRPC.SNAPSHOT || rpc_type == GRZDuelRPC.NOTICE)
  {
   if (g_Game.IsServer() || sender || this != PlayerBase.Cast(g_Game.GetPlayer())) return;
   if (rpc_type == GRZDuelRPC.SNAPSHOT)
   {
    GRZDuelSnapshot data;
    if (ctx.Read(data) && data) { GRZDuelClientBus.Data = data; GRZDuelClientBus.ReceivedAt = g_Game.GetTime(); GRZDuelClientBus.Changed.Invoke(); }
   }
   else
   {
    string message;
    if (ctx.Read(message)) GRZDuelClientBus.Notice.Invoke(message);
   }
   return;
  }
  super.OnRPC(sender, rpc_type, ctx);
 }
};
