class GRZDuelRPC
{
 static const int REQUEST = 763600;
 static const int CHALLENGE = 763601;
 static const int ANSWER = 763602;
 static const int SNAPSHOT = 763603;
 static const int NOTICE = 763604;
 static const int MENU = 763605;
 static const int LEAVE = 763606;
};
class GRZDuelMap
{
 string Id;
 string Name;
 bool Enabled;
 bool GroundSpawns;
 bool SurfaceSpawns;
 vector Center;
 float Radius = 100;
 vector SpawnA;
 vector SpawnB;
 float FacingA;
 float FacingB = 180;
};
class GRZDuelSettings
{
 bool Enabled = true;
 int ChallengeSeconds = 60;
 int CountdownSeconds = 5;
 int RespawnWaitSeconds = 90;
 int RoundSeconds = 180;
 ref array<ref GRZDuelMap> Maps = new array<ref GRZDuelMap>;
};
class GRZDuelPlayerRow
{
 string Id;
 string Name;
 string Weapon;
 int Elo = 800;
 bool Available;
};
class GRZDuelMapRow
{
 string Id;
 string Name;
 bool Busy;
};
class GRZDuelSnapshot
{
 ref array<ref GRZDuelPlayerRow> Players = new array<ref GRZDuelPlayerRow>;
 ref array<ref GRZDuelMapRow> Maps = new array<ref GRZDuelMapRow>;
 int Elo = 800;
 string PendingId;
 string PendingName;
 string PendingMap;
 string PendingGun;
 int PendingBestOf;
 int PendingSeconds;
 bool Incoming;
 bool InMatch;
 int Phase;
 bool AutoRespawn;
 int ScoreSelf;
 int ScoreOpponent;
 int BestOf;
 string Opponent;
 string MapName;
 string Status;
};
class GRZDuelAttachment
{
 string Type;
 int Parent = -1;
 int Slot;
};
class GRZDuelKit
{
 ref GRZLoadoutGun Gun;
 ref array<ref GRZDuelAttachment> Attachments = new array<ref GRZDuelAttachment>;
};
class GRZDuelRating
{
 string SteamId;
 string Name;
 int Kills;
 int Deaths;
 int Elo = 800;
 int Wins;
 int Losses;
};
class GRZDuelRatings
{
 int Revision;
 ref array<ref GRZDuelRating> Players = new array<ref GRZDuelRating>;
};
class GRZDuelRanks
{
 static int Tier(int elo)
 {
  if (elo < 1000) return 0;
  if (elo < 1200) return 1;
  if (elo < 1400) return 2;
  if (elo < 1600) return 3;
  if (elo < 1800) return 4;
  if (elo < 2000) return 5;
  if (elo < 2200) return 6;
  return 7;
 }
 static string Name(int elo)
 {
  array<string> names = {"BRONZE", "SILVER", "GOLD", "PLATINUM", "DIAMOND", "MASTER", "GRANDMASTER", "LEGEND"};
  return names[Tier(elo)];
 }
 static int Color(int elo)
 {
  array<int> colors = {0xFFCD8B56,0xFFCAD4E2,0xFFFFCF54,0xFF67DCCF,0xFF71C8FF,0xFFB18CFF,0xFFFF667E,0xFFFFA34E};
  return colors[Tier(elo)];
 }
 static int Gain(int winnerElo, int loserElo, int k = 32)
 {
  float expected = 1.0 / (1.0 + Math.Pow(10.0, (loserElo - winnerElo) / 400.0));
  return Math.Clamp(Math.Round(k * (1.0 - expected)), 1, k);
 }
};
class GRZDuelClientBus
{
 static ref ScriptInvoker Changed = new ScriptInvoker;
 static ref ScriptInvoker Notice = new ScriptInvoker;
 static ref GRZDuelSnapshot Data;
 static int ReceivedAt;
};

class GRZLeaderboardRPC
{
 static const int REQUEST = 763620;
 static const int PAGE = 763621;
 static const int MENU = 763622;
};
class GRZLeaderboardRow
{
 string Name;
 int Kills;
 int Deaths;
 int Elo;
};
class GRZLeaderboardPage
{
 int Page;
 int Total;
 int Sort;
 int MyElo = 800;
 string Status;
 ref array<ref GRZLeaderboardRow> Rows = new array<ref GRZLeaderboardRow>;
};
class GRZLeaderboardBus
{
 static ref GRZLeaderboardPage Data;
 static ref ScriptInvoker Changed = new ScriptInvoker;
 static string Ratio(int kills, int deaths)
 {
  float ratio = kills / Math.Max(1.0, deaths);
  int scaled = Math.Round(ratio * 100.0);
  int whole = scaled / 100;
  int fraction = scaled % 100;
  return whole.ToString() + "." + fraction.ToStringLen(2);
 }
};
