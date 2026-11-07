class GRZDuelRatingStore
{
 static ref GRZDuelRatings Data;
 static ref map<string,ref GRZDuelRating> Index = new map<string,ref GRZDuelRating>;
 static bool Writable;
 static bool Dirty;
 static int LastFlush;
 static const string FILE = "$profile:GRZ_Duels/ratings.json";
 static bool Valid(GRZDuelRatings data)
 {
  if (!data || !data.Players) return false;
  map<string,bool> seen = new map<string,bool>;
  foreach (GRZDuelRating row : data.Players)
  {
   if (!row || row.SteamId == "" || row.Elo < 0 || row.Wins < 0 || row.Losses < 0 || row.Kills < 0 || row.Deaths < 0 || seen.Contains(row.SteamId)) return false;
   seen.Insert(row.SteamId,true);
  }
  return true;
 }
 static bool Same(GRZDuelRatings a, GRZDuelRatings b)
 {
  if (!Valid(a) || !Valid(b) || a.Revision != b.Revision || a.Players.Count() != b.Players.Count()) return false;
  for (int i = 0; i < a.Players.Count(); i++)
  {
   GRZDuelRating x = a.Players[i]; GRZDuelRating y = b.Players[i];
   if (x.SteamId != y.SteamId || x.Name != y.Name || x.Elo != y.Elo || x.Wins != y.Wins || x.Losses != y.Losses || x.Kills != y.Kills || x.Deaths != y.Deaths) return false;
  }
  return true;
 }
 static void Init()
 {
  if (Data) return;
  MakeDirectory("$profile:GRZ_Duels");
  string error;
  Writable = true;
  if (FileExist(FILE))
  {
   if (JsonFileLoader<GRZDuelRatings>.LoadFile(FILE, Data, error) && Valid(Data)) { Reindex(); return; }
   if (JsonFileLoader<GRZDuelRatings>.LoadFile(FILE + ".backup", Data, error) && Valid(Data))
   {
    if (!CopyFile(FILE + ".backup", FILE)) Writable = false;
    Reindex(); return;
   }
   Writable = false;
   Print("[GRZ Duels] Ratings/stats cannot be read. Ranked duels disabled; existing files preserved.");
  }
  Data = new GRZDuelRatings;
  Reindex();
 }
 static void Reindex()
 {
  Index.Clear();
  foreach (GRZDuelRating row : Data.Players) Index.Insert(row.SteamId,row);
 }
 static GRZDuelRatings Clone()
 {
  GRZDuelRatings next = new GRZDuelRatings;
  next.Revision = Data.Revision + 1;
  foreach (GRZDuelRating row : Data.Players)
  {
   GRZDuelRating copy = new GRZDuelRating;
   copy.SteamId = row.SteamId; copy.Name = row.Name;
   copy.Elo = row.Elo; copy.Wins = row.Wins; copy.Losses = row.Losses;
   copy.Kills = row.Kills; copy.Deaths = row.Deaths;
   next.Players.Insert(copy);
  }
  return next;
 }
 static bool Commit(GRZDuelRatings next)
 {
  if (!Writable) return false;
  string error;
  GRZDuelRatings verify;
  if (!JsonFileLoader<GRZDuelRatings>.SaveFile(FILE + ".pending", next, error)) { Writable = false; return false; }
  if (!JsonFileLoader<GRZDuelRatings>.LoadFile(FILE + ".pending", verify, error) || !Same(next, verify)) { Writable = false; return false; }
  if (FileExist(FILE) && !CopyFile(FILE, FILE + ".backup")) { Writable = false; return false; }
  if (!CopyFile(FILE + ".pending", FILE)) { Writable = false; return false; }
  if (!JsonFileLoader<GRZDuelRatings>.LoadFile(FILE, verify, error) || !Same(next, verify))
  {
   if (FileExist(FILE + ".backup")) CopyFile(FILE + ".backup", FILE);
   Writable = false;
   return false;
  }
  Data = next; Dirty = false; Reindex();
  DeleteFile(FILE + ".pending");
  return true;
 }
 static int Elo(string steamId)
 {
  Init();
  GRZDuelRating row;
  if (Index.Find(steamId,row)) return row.Elo;
  return 800;
 }
 static GRZDuelRating Upsert(GRZDuelRatings data, string steamId)
 {
  if (data == Data)
  {
   GRZDuelRating indexed;
   if (Index.Find(steamId,indexed)) return indexed;
  }
  else foreach (GRZDuelRating row : data.Players) if (row.SteamId == steamId) return row;
  GRZDuelRating created = new GRZDuelRating;
  created.SteamId = steamId; data.Players.Insert(created);
  if (data == Data) { Index.Insert(steamId,created); Dirty = true; }
  return created;
 }
 static void Touch(PlayerBase p)
 {
  Init();
  if (!Writable || !p || !p.GetIdentity() || p.GetIdentity().GetPlainId() == "") return;
  GRZDuelRating row = Upsert(Data,p.GetIdentity().GetPlainId());
  if (row.Name != p.GetIdentity().GetName()) { row.Name = p.GetIdentity().GetName(); Dirty = true; }
 }
 static void Death(PlayerBase victim, PlayerBase killer)
 {
  Init();
  if (!Writable) return;
  int victimDeaths;
  int killerKills;
  if (victim && victim.GetIdentity() && victim.GetIdentity().GetPlainId() != "")
  {
   Touch(victim);
   GRZDuelRating dead = Upsert(Data,victim.GetIdentity().GetPlainId()); dead.Deaths++; victimDeaths = dead.Deaths;
   Dirty = true;
  }
  if (killer && killer != victim && killer.GetIdentity() && killer.GetIdentity().GetPlainId() != "")
  {
   Touch(killer);
   GRZDuelRating won = Upsert(Data,killer.GetIdentity().GetPlainId()); won.Kills++; killerKills = won.Kills;
   Dirty = true;
  }
  PlayerIdentity statsKiller;
  PlayerIdentity statsVictim;
  string counters = "Live counters; periodic persistence pending";
  if (victim && victim.GetIdentity())
  {
   statsVictim = victim.GetIdentity();
   counters += " | Victim deaths: " + victimDeaths.ToString();
  }
  if (killer && killer != victim && killer.GetIdentity())
  {
   statsKiller = killer.GetIdentity();
   counters += " | Killer kills: " + killerKills.ToString();
  }
  GRZAudit.Record("leaderboard_stats", statsKiller, counters, statsVictim);
 }
 static bool Result(string winner, string loser, out int delta)
 {
  Init();
  if (!Writable || winner == "" || loser == "" || winner == loser) return false;
  GRZDuelRatings next = Clone();
  GRZDuelRating win = Upsert(next, winner);
  GRZDuelRating loss = Upsert(next, loser);
  delta = GRZDuelRanks.Gain(win.Elo, loss.Elo);
  int priorElo = win.Elo;
  win.Elo += delta; loss.Elo = Math.Max(0, loss.Elo - delta);
  win.Wins++; loss.Losses++;
  if (!Commit(next)) return false;
  GRZAuditEvent audit = new GRZAuditEvent;
  audit.Kind = "leaderboard_update"; audit.PlayerId = winner; audit.PlayerName = win.Name;
  audit.OtherId = loser; audit.OtherName = loss.Name;
  audit.Details = "Rated duel saved | Winner Elo: " + win.Elo.ToString() + " (+" + delta.ToString() + ") | Loser Elo: " + loss.Elo.ToString();
  GRZAudit.Send(audit);
  if (GRZDuelRanks.Tier(win.Elo) > GRZDuelRanks.Tier(priorElo))
  {
   GRZAuditEvent promotion = new GRZAuditEvent;
   promotion.Kind = "rank_up"; promotion.PlayerId = winner; promotion.PlayerName = win.Name;
   promotion.Details = GRZDuelRanks.Name(priorElo) + " -> " + GRZDuelRanks.Name(win.Elo) + " | Elo: " + win.Elo.ToString();
   GRZAudit.Send(promotion);
  }
  return true;
 }
 static void Flush(bool force = false)
 {
  if (!Data || !Dirty || !Writable) return;
  if (!force && g_Game.GetTime() - LastFlush < 10000) return;
  LastFlush = g_Game.GetTime();
  if (!Commit(Clone())) Print("[GRZ Duels] Cannot save ratings/stats. Further ranked updates disabled until storage is repaired.");
 }
};
