// Server-authoritative events only. No tokens or Discord requests in the game.
class GRZAuditEvent
{
 int Schema = 1;
 string Id;
 string Timestamp;
 string Kind;
 string PlayerId;
 string PlayerName;
 string OtherId;
 string OtherName;
 string Details;
 bool Vip;
};
class GRZAudit
{
 protected static string Session;
 protected static int Sequence;
 protected static bool Warned;
 static string Stamp()
 {
  int year, month, day, hour, minute, second;
  GetYearMonthDayUTC(year, month, day);
  GetHourMinuteSecondUTC(hour, minute, second);
  return year.ToString() + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2) + "T" + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2) + "Z";
 }
 static void Record(string kind, PlayerIdentity actor = null, string details = "", PlayerIdentity other = null, bool vip = false)
 {
  GRZAuditEvent entry = new GRZAuditEvent;
  entry.Kind = kind; entry.Details = details; entry.Vip = vip;
  if (actor) { entry.PlayerId = actor.GetPlainId(); entry.PlayerName = actor.GetName(); }
  if (other) { entry.OtherId = other.GetPlainId(); entry.OtherName = other.GetName(); }
  Send(entry);
 }
 static void Send(GRZAuditEvent entry)
 {
  if (!g_Game || !g_Game.IsServer() || !entry) return;
  if (Session == "")
  {
   string sessionTime = Stamp();
   sessionTime.Replace(":", "-");
   Session = sessionTime + "-" + Math.RandomInt(100000000, 2000000000).ToString();
  }
  Sequence++;
  entry.Id = Session + "-" + Sequence.ToString();
  entry.Timestamp = Stamp();
  string encoded;
  JsonSerializer serializer = new JsonSerializer;
  if (!serializer.WriteToString(entry, false, encoded)) { Failed(); return; }
  MakeDirectory("$profile:GRZ_Logs");
  FileHandle file = OpenFile("$profile:GRZ_Logs/events-" + Session + ".jsonl", FileMode.APPEND);
  if (!file) { Failed(); return; }
  FPrintln(file, encoded);
  CloseFile(file);
 }
 protected static void Failed()
 {
  if (Warned) return;
  Warned = true;
  Print("[GRZ Audit] Cannot write audit events. Check profiles/GRZ_Logs permissions and free space. Gameplay continues.");
 }
};
