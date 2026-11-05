class GRZLeaderboardCache
{
 int Sort;
 int BuiltAt;
 ref array<ref GRZLeaderboardRow> Rows = new array<ref GRZLeaderboardRow>;
};
class GRZLeaderboardServer
{
 static ref array<ref GRZLeaderboardCache> Caches = new array<ref GRZLeaderboardCache>;
 static GRZLeaderboardCache Sorted(int sort)
 {
  GRZDuelRatingStore.Init();
  GRZLeaderboardCache cache;
  foreach (GRZLeaderboardCache entry : Caches) if (entry.Sort == sort) { cache = entry; break; }
  if (cache && g_Game.GetTime() - cache.BuiltAt < 5000) return cache;
  if (!cache) { cache = new GRZLeaderboardCache; cache.Sort = sort; Caches.Insert(cache); }
  cache.BuiltAt = g_Game.GetTime(); cache.Rows.Clear();
  array<string> keys = new array<string>;
  map<string,ref GRZLeaderboardRow> rows = new map<string,ref GRZLeaderboardRow>;
  foreach (GRZDuelRating rating : GRZDuelRatingStore.Data.Players)
  {
   GRZLeaderboardRow row = new GRZLeaderboardRow;
   row.Name = rating.Name; if (row.Name == "") row.Name = "Player";
   row.Kills = rating.Kills; row.Deaths = rating.Deaths; row.Elo = rating.Elo;
   int primary = row.Kills;
   if (sort == 1) primary = Math.Clamp(row.Kills / Math.Max(1.0,row.Deaths) * 10000.0,0,2000000000);
   if (sort == 2) primary = row.Elo;
   int lowerDeaths = 2000000000 - Math.Min(2000000000,row.Deaths);
   string key = primary.ToStringLen(10) + "|" + row.Kills.ToStringLen(10) + "|" + lowerDeaths.ToStringLen(10) + "|" + rating.SteamId;
   keys.Insert(key); rows.Insert(key,row);
  }
  keys.Sort(true);
  foreach (string sorted : keys) cache.Rows.Insert(rows[sorted]);
  return cache;
 }
 static void Page(PlayerBase p, int page, int sort)
 {
  GRZDuelServer.Init();
  GRZDuelRatingStore.Touch(p);
  sort = Math.Clamp(sort,0,2);
  GRZLeaderboardCache cache = Sorted(sort);
  GRZLeaderboardPage result = new GRZLeaderboardPage;
  result.Total = cache.Rows.Count(); result.Sort = sort;
  result.MyElo = GRZDuelRatingStore.Elo(p.GetIdentity().GetPlainId());
  result.Page = Math.Clamp(page,0,Math.Max(0,(result.Total - 1) / 20));
  if (!GRZDuelRatingStore.Writable) result.Status = "Leaderboard is temporarily unavailable. Ask a server admin.";
  int start = result.Page * 20;
  for (int i = start; i < Math.Min(start + 20,result.Total); i++) result.Rows.Insert(cache.Rows[i]);
  ScriptRPC rpc = new ScriptRPC;
  rpc.Write(result); rpc.Send(p,GRZLeaderboardRPC.PAGE,true,p.GetIdentity());
 }
};
modded class PlayerBase
{
 protected int m_GRZLeaderboardRequest = -10000;
 override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
 {
  if (rpc_type == GRZLeaderboardRPC.REQUEST)
  {
   if (!g_Game.IsServer() || !sender || !GetIdentity() || sender.GetId() != GetIdentity().GetId()) return;
   if (g_Game.GetTime() - m_GRZLeaderboardRequest < 1000) return;
   m_GRZLeaderboardRequest = g_Game.GetTime();
   int page; int sort;
   if (ctx.Read(page) && ctx.Read(sort)) GRZLeaderboardServer.Page(this,page,sort);
   return;
  }
  if (rpc_type == GRZLeaderboardRPC.PAGE)
  {
   if (g_Game.IsServer() || sender || this != PlayerBase.Cast(g_Game.GetPlayer())) return;
   GRZLeaderboardPage data;
   if (ctx.Read(data) && data) { GRZLeaderboardBus.Data = data; GRZLeaderboardBus.Changed.Invoke(); }
   return;
  }
  super.OnRPC(sender,rpc_type,ctx);
 }
};
