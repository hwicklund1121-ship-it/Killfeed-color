class GRZRotationServer
{
 static ref GRZRotationSettings Settings;
 static ref map<string, string> Votes = new map<string, string>;
 static bool Enabled;
 static bool Voting;
 static int ActiveIndex;
 static int RoundId;
 static int RoundStart;
 static bool VoteWarning;
 static bool ChangeWarning;
 static bool VoteLocked;
 static int NextIndex = -1;
 static int LastBroadcast;

 static bool Inside(vector position, GRZRotationArena arena)
 {
  bool inside = false;
  int j = arena.Boundary.Count() - 1;
  for (int i = 0; i < arena.Boundary.Count(); i++)
  {
   GRZRotationPoint a = arena.Boundary[i];
   GRZRotationPoint b = arena.Boundary[j];
   if ((a.Z > position[2]) != (b.Z > position[2]))
   {
    float crossing = (b.X - a.X) * (position[2] - a.Z) / (b.Z - a.Z) + a.X;
    if (position[0] < crossing)
     inside = !inside;
   }
   j = i;
  }
  return inside;
 }

 static bool SafeSpawn(GRZRotationPoint point, GRZRotationArena arena, out vector position)
 {
  if (g_Game.SurfaceIsSea(point.X, point.Z) || g_Game.SurfaceIsPond(point.X, point.Z))
   return false;
  float ground = g_Game.SurfaceY(point.X, point.Z);
  vector from = Vector(point.X, ground + 2.2, point.Z);
  vector to = Vector(point.X, ground - 0.5, point.Z);
  vector contact;
  vector normal;
  int component;
  set<Object> hits = new set<Object>;
  position = Vector(point.X, ground + 0.15, point.Z);
  if (DayZPhysics.RaycastRV(from, to, contact, normal, component, hits, null, null, false, false, ObjIntersectGeom))
  {
   if (normal[1] < 0.65)
    return false;
   position = contact + "0 0.15 0";
  }
  else
  {
   vector terrainNormal = g_Game.SurfaceGetNormal(point.X, point.Z);
   if (terrainNormal[1] < 0.65)
    return false;
  }
  if (!Inside(position, arena))
   return false;
  array<Object> exclude = new array<Object>;
  vector boxCenter = position + "0 1.05 0";
  if (g_Game.IsBoxCollidingGeometry(boxCenter, "0 0 0", "0.75 1.7 0.75", ObjIntersectView, ObjIntersectGeom, exclude))
   return false;
  return true;
 }

 static void Init()
 {
  Enabled = false;
  Votes.Clear();
  string directory = "$profile:GRZ_MapRotation";
  MakeDirectory(directory);
  string path = directory + "/config.json";
  Settings = new GRZRotationSettings;
  string error;
  if (!FileExist(path))
  {
   if (!JsonFileLoader<GRZRotationSettings>.LoadFile("GRZ_MapRotation/data/default-config.json", Settings, error))
   {
    Error("[GRZ Rotation] Cannot load bundled default config: " + error);
    return;
   }
   if (!JsonFileLoader<GRZRotationSettings>.SaveFile(path, Settings, error))
   {
    Error("[GRZ Rotation] Cannot save profile config: " + error);
    return;
   }
  }
  if (!JsonFileLoader<GRZRotationSettings>.LoadFile(path, Settings, error))
  {
   Error("[GRZ Rotation] Config failed: " + error);
   return;
  }
  if (!Settings || !Settings.Arenas || Settings.Arenas.Count() < 2 || Settings.Arenas.Count() > 6)
  {
   Error("[GRZ Rotation] Configure between two and six arenas.");
   return;
  }
  Settings.RoundSeconds = Math.Clamp(Settings.RoundSeconds, 180, 7200);
  Settings.VoteSeconds = Math.Clamp(Settings.VoteSeconds, 60, Settings.RoundSeconds - 30);
  Settings.PopupSeconds = Math.Clamp(Settings.PopupSeconds, 5, 30);
  Settings.OutsideSeconds = Math.Clamp(Settings.OutsideSeconds, 1, 30);
  if (!g_Game.ConfigIsExisting("CfgPatches EditorLoader_Scripts"))
  {
   Error("[GRZ Rotation] Editor Loader is not loaded. Rotation disabled.");
   return;
  }
  array<string> ids = new array<string>;
  for (int i = 0; i < Settings.Arenas.Count(); i++)
  {
   GRZRotationArena arena = Settings.Arenas[i];
   if (!arena || !arena.Boundary || !arena.SpawnCandidates)
    return;
   if (arena.Id == "" || arena.Boundary.Count() < 3 || ids.Find(arena.Id) >= 0)
   {
    Error("[GRZ Rotation] Invalid or duplicated arena ID/boundary.");
    return;
   }
   ids.Insert(arena.Id);
   arena.SafeSpawns = new array<vector>;
   foreach (GRZRotationPoint candidate : arena.SpawnCandidates)
   {
    vector safe;
    if (SafeSpawn(candidate, arena, safe))
     arena.SafeSpawns.Insert(safe);
   }
   Print(string.Format("[GRZ Rotation] %1: %2 runtime-checked spawn positions.", arena.Name, arena.SafeSpawns.Count()));
   if (arena.SafeSpawns.Count() < 2)
   {
    Error("[GRZ Rotation] Fewer than two valid spawn positions. Fix the arena geometry/spawn config; rotation disabled.");
    return;
   }
  }
  ActiveIndex = ids.Find(Settings.StartArena);
  if (ActiveIndex < 0)
   ActiveIndex = 0;
  GRZRotationState saved = new GRZRotationState;
  int savedIndex = -1;
  if (JsonFileLoader<GRZRotationState>.LoadFile(directory + "/active-arena.json", saved, error) && saved)
   savedIndex = ids.Find(saved.ActiveId);
  if (savedIndex >= 0)
   ActiveIndex = savedIndex;
  Enabled = true;
  BeginRound();
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(Tick);
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, 250, true);
 }

 static int Remaining()
 {
  return Math.Max(0, Settings.RoundSeconds - (g_Game.GetTime() - RoundStart) / 1000);
 }

 static void BeginRound()
 {
  RoundId++;
  RoundStart = g_Game.GetTime();
  Voting = false;
  VoteWarning = false;
  ChangeWarning = false;
  VoteLocked = false;
  NextIndex = -1;
  Votes.Clear();
  GRZRotationState state = new GRZRotationState;
  state.ActiveId = Settings.Arenas[ActiveIndex].Id;
  JsonFileLoader<GRZRotationState>.JsonSaveFile("$profile:GRZ_MapRotation/active-arena.json", state);
 }

 static void SendSnapshot(PlayerBase player)
 {
  if (!player || !player.GetIdentity())
   return;
  GRZRotationSnapshot data = new GRZRotationSnapshot;
  data.Enabled = Enabled;
  if (Enabled)
  {
   data.Voting = Voting;
   data.RoundId = RoundId;
   data.Remaining = Remaining();
   data.PopupSeconds = Settings.PopupSeconds;
   data.ActiveId = Settings.Arenas[ActiveIndex].Id;
   data.ActiveName = Settings.Arenas[ActiveIndex].Name;
   Votes.Find(player.GetIdentity().GetId(), data.OwnVote);
   foreach (GRZRotationArena arena : Settings.Arenas)
   {
    GRZRotationChoice choice = new GRZRotationChoice;
    choice.Id = arena.Id;
    choice.Name = arena.Name;
    choice.Blueprint = arena.Blueprint;
    for (int v = 0; v < Votes.Count(); v++)
    {
     if (Votes.GetElement(v) == arena.Id)
      choice.Votes++;
    }
    data.Choices.Insert(choice);
   }
  }
  ScriptRPC rpc = new ScriptRPC;
  rpc.Write(data);
  rpc.Send(player, GRZRotationRPC.SNAPSHOT, true, player.GetIdentity());
 }

 static void Notice(PlayerBase player, string message, int sound)
 {
  if (!player || !player.GetIdentity())
   return;
  ScriptRPC rpc = new ScriptRPC;
  rpc.Write(message);
  rpc.Write(sound);
  rpc.Send(player, GRZRotationRPC.NOTICE, true, player.GetIdentity());
 }

 static void BroadcastNotice(string message, int sound)
 {
  array<Man> players = new array<Man>;
  g_Game.GetPlayers(players);
  foreach (Man man : players)
   Notice(PlayerBase.Cast(man), message, sound);
 }

 static void Broadcast()
 {
  array<Man> players = new array<Man>;
  g_Game.GetPlayers(players);
  foreach (Man man : players)
   SendSnapshot(PlayerBase.Cast(man));
 }

 static void CastVote(PlayerBase player, int round, string arenaId)
 {
  if (!Enabled || !Voting || round != RoundId || Remaining() <= 30)
  {
   Notice(player, "Voting is closed.", 0);
   SendSnapshot(player);
   return;
  }
  for (int i = 0; i < Settings.Arenas.Count(); i++)
  {
   if (Settings.Arenas[i].Id != arenaId || i == ActiveIndex)
    continue;
   Votes.Set(player.GetIdentity().GetId(), arenaId);
   Broadcast();
   return;
  }
 }

 static int Winner()
 {
  int best = 0;
  array<int> winners = new array<int>;
  for (int i = 0; i < Settings.Arenas.Count(); i++)
  {
   if (i == ActiveIndex)
    continue;
   int count = 0;
   for (int v = 0; v < Votes.Count(); v++)
   {
    if (Votes.GetElement(v) == Settings.Arenas[i].Id)
     count++;
   }
   if (count > best)
   {
    best = count;
    winners.Clear();
   }
   if (count == best)
    winners.Insert(i);
  }
  if (best == 0)
   return (ActiveIndex + 1) % Settings.Arenas.Count();
  return winners[Math.RandomInt(0, winners.Count())];
 }

 static bool MovePlayer(PlayerBase player)
 {
  if (!Enabled || !player || !player.IsAlive())
   return false;
  if (player.GetCommand_Vehicle())
  {
   if (g_Game.GetTime() - player.m_GRZVehicleExitTime >= 5000)
   {
    player.m_GRZVehicleExitTime = g_Game.GetTime();
    player.GetCommand_Vehicle().GetOutVehicle();
    g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(MovePlayer, 2000, false, player);
   }
   return false;
  }
  GRZRotationArena arena = Settings.Arenas[ActiveIndex];
  array<Man> players = new array<Man>;
  g_Game.GetPlayers(players);
  vector bestPosition = arena.SafeSpawns.GetRandomElement();
  float bestDistance = -1;
  int offset = Math.RandomInt(0, arena.SafeSpawns.Count());
  for (int i = 0; i < arena.SafeSpawns.Count(); i++)
  {
   vector candidate = arena.SafeSpawns[(i + offset) % arena.SafeSpawns.Count()];
   float nearest = 1000000;
   foreach (Man man : players)
   {
    if (man == player || !man.IsAlive())
     continue;
    float distance = vector.DistanceSq(candidate, man.GetPosition());
    nearest = Math.Min(nearest, distance);
   }
   if (nearest > bestDistance)
   {
    bestPosition = candidate;
    bestDistance = nearest;
   }
  }
  player.SetPosition(bestPosition);
  player.SetOrientation("0 0 0");
  player.m_GRZOutsideStart = -1;
  player.m_GRZOutsideLastCount = -1;
  Notice(player, "", 0);
  return true;
 }

 static void PlayerReady(PlayerBase player)
 {
  if (!player || !player.GetIdentity())
   return;
  player.m_GRZRotationReady = true;
  if (Enabled)
  {
   MovePlayer(player);
   SendSnapshot(player);
  }
 }

 static void Tick()
 {
  if (!Enabled)
   return;
  int remaining = Remaining();
  if (!VoteWarning && remaining <= Settings.VoteSeconds + 30)
  {
   VoteWarning = true;
   BroadcastNotice("Map voting opens in 30 seconds.", 2);
  }
  if (!Voting && !VoteLocked && remaining <= Settings.VoteSeconds)
  {
   Voting = true;
   BroadcastNotice("Map voting is open. Type !vote to vote or change your vote.", 1);
   Broadcast();
  }
  if (!ChangeWarning && remaining <= 30)
  {
   ChangeWarning = true;
   NextIndex = Winner();
   Voting = false;
   VoteLocked = true;
   BroadcastNotice("Vote closed. " + Settings.Arenas[NextIndex].Name + " starts in 30 seconds.", 2);
   Broadcast();
  }
  if (remaining <= 0)
  {
   if (NextIndex >= 0)
    ActiveIndex = NextIndex;
   else
    ActiveIndex = Winner();
   BeginRound();
   array<Man> rotating = new array<Man>;
   g_Game.GetPlayers(rotating);
   foreach (Man participant : rotating)
   {
    PlayerBase nextPlayer = PlayerBase.Cast(participant);
    if (nextPlayer && nextPlayer.GetIdentity() && nextPlayer.m_GRZRotationReady)
     MovePlayer(nextPlayer);
   }
   BroadcastNotice("Arena changed to " + Settings.Arenas[ActiveIndex].Name + ".", 3);
   Broadcast();
  }
  array<Man> players = new array<Man>;
  g_Game.GetPlayers(players);
  foreach (Man man : players)
  {
   PlayerBase player = PlayerBase.Cast(man);
   if (!player || !player.GetIdentity() || !player.IsAlive() || !player.m_GRZRotationReady)
    continue;
   if (Inside(player.GetPosition(), Settings.Arenas[ActiveIndex]))
   {
    if (player.m_GRZOutsideStart >= 0)
     Notice(player, "", 0);
    player.m_GRZOutsideStart = -1;
    player.m_GRZOutsideLastCount = -1;
    continue;
   }
   if (player.m_GRZOutsideStart < 0)
    player.m_GRZOutsideStart = g_Game.GetTime();
   int elapsed = (g_Game.GetTime() - player.m_GRZOutsideStart) / 1000;
   if (elapsed >= Settings.OutsideSeconds)
    MovePlayer(player);
   else if (player.m_GRZOutsideLastCount != Settings.OutsideSeconds - elapsed)
   {
    player.m_GRZOutsideLastCount = Settings.OutsideSeconds - elapsed;
    Notice(player, string.Format("Outside arena — returning in %1 seconds.", player.m_GRZOutsideLastCount), 0);
   }
  }
  if (g_Game.GetTime() - LastBroadcast >= 5000)
  {
   LastBroadcast = g_Game.GetTime();
   Broadcast();
  }
 }

 static void Stop()
 {
  Enabled = false;
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(Tick);
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(MovePlayer);
 }
};

modded class PlayerBase
{
 int m_GRZOutsideStart = -1;
 int m_GRZOutsideLastCount = -1;
 bool m_GRZRotationReady;
 int m_GRZVehicleExitTime = -10000;
 protected int m_GRZRotationRequestTime = -10000;
 protected int m_GRZRotationVoteTime = -10000;

 override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
 {
  if (rpc_type == GRZRotationRPC.REQUEST || rpc_type == GRZRotationRPC.VOTE)
  {
   if (!g_Game.IsServer() || !sender || !GetIdentity())
    return;
   if (sender.GetId() != GetIdentity().GetId())
    return;
   int now = g_Game.GetTime();
   if (rpc_type == GRZRotationRPC.REQUEST)
   {
    if (now - m_GRZRotationRequestTime < 1000)
     return;
    m_GRZRotationRequestTime = now;
    GRZRotationServer.SendSnapshot(this);
   }
   else
   {
    if (now - m_GRZRotationVoteTime < 500)
     return;
    int round;
    string arena;
    if (!ctx.Read(round))
     return;
    if (!ctx.Read(arena))
     return;
    m_GRZRotationVoteTime = now;
    GRZRotationServer.CastVote(this, round, arena);
   }
   return;
  }
  if (rpc_type == GRZRotationRPC.SNAPSHOT || rpc_type == GRZRotationRPC.NOTICE)
  {
   if (g_Game.IsServer() || this != g_Game.GetPlayer())
    return;
   if (rpc_type == GRZRotationRPC.SNAPSHOT)
   {
    GRZRotationSnapshot data;
    if (ctx.Read(data) && data)
     GRZRotationBus.Snapshot.Invoke(data);
   }
   else
   {
    string message;
    int sound;
    if (!ctx.Read(message))
     return;
    if (!ctx.Read(sound))
     return;
    GRZRotationBus.Notice.Invoke(message, sound);
   }
   return;
  }
  super.OnRPC(sender, rpc_type, ctx);
 }
};
