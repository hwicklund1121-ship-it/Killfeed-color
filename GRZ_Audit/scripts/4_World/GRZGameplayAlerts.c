// Visible server gameplay diagnostics. No keyboard hooks, client RPCs or punishments.
class GRZGameplayAlertSettings
{
 bool Enabled = true;
 bool MovementEnabled = true;
 bool FireRateEnabled = true;
 bool DamageEnabled = true;
 float MaxOnFootMetresPerSecond = 16.0;
 float PositionJumpMetres = 120.0;
 int ConsecutiveSpeedSamples = 3;
 int SpawnGraceSeconds = 20;
 int CooldownSeconds = 60;
 float FireIntervalRatio = 0.55;
 int ConsecutiveFireWindows = 2;
 float DamageEnvelopeMultiplier = 8.0;
 ref array<string> ExemptSteamIds = new array<string>;
};
class GRZGameplayAlertState
{
 int GraceUntil;
 int MoveAt;
 vector Position;
 int SpeedSamples;
 int SpeedAlertAt = -600000;
 int JumpAlertAt = -600000;
 int FireAlertAt = -600000;
 int DamageAlertAt = -600000;
 int DamageWindowAt;
 int DamageSamples;
};
class GRZGameplayAlerts
{
 static ref GRZGameplayAlertSettings Settings;
 static void Init()
 {
  if (Settings || !g_Game || !g_Game.IsServer()) return;
  Settings = new GRZGameplayAlertSettings;
  string path = "$profile:GRZ_Logs/gameplay-alerts.json";
  MakeDirectory("$profile:GRZ_Logs");
  string error;
  if (FileExist(path))
  {
   if (!JsonFileLoader<GRZGameplayAlertSettings>.LoadFile(path, Settings, error))
   {
    Settings = new GRZGameplayAlertSettings;
    Settings.Enabled = false;
    Print("[GRZ Gameplay Alerts] Invalid config; alerts disabled. Fix profiles/GRZ_Logs/gameplay-alerts.json and restart.");
    return;
   }
  }
  else if (!JsonFileLoader<GRZGameplayAlertSettings>.SaveFile(path, Settings, error))
   Print("[GRZ Gameplay Alerts] Cannot save settings; using defaults for this session.");
  Settings.MaxOnFootMetresPerSecond = Math.Clamp(Settings.MaxOnFootMetresPerSecond, 10.0, 100.0);
  Settings.PositionJumpMetres = Math.Clamp(Settings.PositionJumpMetres, 50.0, 10000.0);
  Settings.ConsecutiveSpeedSamples = Math.Clamp(Settings.ConsecutiveSpeedSamples, 3, 30);
  Settings.SpawnGraceSeconds = Math.Clamp(Settings.SpawnGraceSeconds, 10, 300);
  Settings.CooldownSeconds = Math.Clamp(Settings.CooldownSeconds, 30, 3600);
  Settings.FireIntervalRatio = Math.Clamp(Settings.FireIntervalRatio, 0.1, 0.75);
  Settings.ConsecutiveFireWindows = Math.Clamp(Settings.ConsecutiveFireWindows, 2, 10);
  Settings.DamageEnvelopeMultiplier = Math.Clamp(Settings.DamageEnvelopeMultiplier, 4.0, 100.0);
  if (!Settings.ExemptSteamIds) Settings.ExemptSteamIds = new array<string>;
  GRZAudit.Record("gameplay_alert_status", null, "Server gameplay alerts initialized. Review flags only; no automatic bans. Enabled: " + Settings.Enabled.ToString());
 }
 static GRZGameplayAlertState State(PlayerBase player)
 {
  if (!g_Game || !g_Game.IsServer() || !player) return null;
  Init();
  if (!Settings || !Settings.Enabled || !player.GetIdentity()) return null;
  if (Settings.ExemptSteamIds.Find(player.GetIdentity().GetPlainId()) >= 0) return null;
  if (!player.m_GRZGameplayAlerts)
  {
   player.m_GRZGameplayAlerts = new GRZGameplayAlertState;
   player.m_GRZGameplayAlerts.GraceUntil = g_Game.GetTime() + Settings.SpawnGraceSeconds * 1000;
   player.m_GRZGameplayAlerts.MoveAt = g_Game.GetTime();
   player.m_GRZGameplayAlerts.Position = player.GetPosition();
  }
  return player.m_GRZGameplayAlerts;
 }
 // Call BEFORE a legitimate server/admin teleport. GRZ rotation and duels do this.
 static void AllowTeleport(PlayerBase player, int seconds = 10)
 {
  GRZGameplayAlertState state = State(player);
  if (!state) return;
  state.GraceUntil = Math.Max(state.GraceUntil, g_Game.GetTime() + seconds * 1000);
  state.SpeedSamples = 0;
 }
 static void Report(string kind, PlayerBase player, string evidence, PlayerBase target = null)
 {
  PlayerIdentity other;
  if (target) other = target.GetIdentity();
  GRZAudit.Record(kind, player.GetIdentity(), "REVIEW FLAG; not proof of cheating. " + evidence, other);
 }
 static bool Paused(PlayerBase player)
 {
  if (!player.IsAlive() || player.IsUnconscious() || player.GetCommand_Vehicle()) return true;
  if (player.GetCommand_Ladder() || player.GetCommand_Climb() || player.GetCommand_Swim()) return true;
  return player.IsFalling();
 }
 static void Sample(PlayerBase player)
 {
  GRZGameplayAlertState state = State(player);
  if (!state || !Settings.MovementEnabled) return;
  int now = g_Game.GetTime();
  int elapsed = now - state.MoveAt;
  vector position = player.GetPosition();
  vector previous = state.Position;
  state.MoveAt = now; state.Position = position;
  if (now < state.GraceUntil || Paused(player) || !GRZAlertRules.ValidSample(elapsed))
  {
   state.SpeedSamples = 0;
   return;
  }
  vector horizontal = position - previous;
  horizontal[1] = 0;
  float distance = horizontal.Length();
  float speed = distance * 1000.0 / elapsed;
  string evidence = "Horizontal speed " + speed.ToString() + " m/s | Distance " + distance.ToString() + " m in " + elapsed.ToString() + " ms";
  evidence += " | From " + previous.ToString() + " to " + position.ToString();
  if (distance > Settings.PositionJumpMetres)
  {
   state.SpeedSamples = 0;
   if (now - state.JumpAlertAt >= Settings.CooldownSeconds * 1000)
   {
    state.JumpAlertAt = now;
    Report("gameplay_position_jump", player, evidence + " | No GRZ teleport exemption. Admin teleports/replication may explain this.");
   }
   return;
  }
  if (GRZAlertRules.FastMovement(distance, elapsed, Settings.MaxOnFootMetresPerSecond)) state.SpeedSamples++;
  else state.SpeedSamples = 0;
  if (state.SpeedSamples >= Settings.ConsecutiveSpeedSamples && now - state.SpeedAlertAt >= Settings.CooldownSeconds * 1000)
  {
   state.SpeedAlertAt = now;
   Report("gameplay_speed", player, evidence + " | Limit " + Settings.MaxOnFootMetresPerSecond.ToString() + " m/s | Consecutive samples " + state.SpeedSamples.ToString());
  }
 }
 static void Tick()
 {
  if (!g_Game || !g_Game.IsServer() || !Settings || !Settings.Enabled) return;
  array<Man> players = new array<Man>;
  g_Game.GetPlayers(players);
  foreach (Man man : players)
  {
   PlayerBase player = PlayerBase.Cast(man);
   if (player) Sample(player);
  }
 }
 static void Damage(PlayerBase target, TotalDamageResult result, int damageType, EntityAI source, string zone, string ammo)
 {
  if (!g_Game || !g_Game.IsServer() || !result || !source || damageType != DamageType.FIRE_ARM) return;
  PlayerBase attacker = PlayerBase.Cast(source.GetHierarchyRootPlayer());
  if (!attacker || attacker == target) return;
  GRZGameplayAlertState state = State(attacker);
  if (!state || !Settings.DamageEnabled || g_Game.GetTime() < state.GraceUntil) return;
  string config = "CfgAmmo " + ammo + " DamageApplied Health damage";
  if (!g_Game.ConfigIsExisting(config)) return;
  float nominal = g_Game.ConfigGetFloat(config);
  float observed = result.GetDamage(zone, "Health");
  if (!GRZAlertRules.ExcessDamage(observed, nominal, Settings.DamageEnvelopeMultiplier)) return;
  int now = g_Game.GetTime();
  if (now - state.DamageWindowAt > 10000) state.DamageSamples = 0;
  state.DamageWindowAt = now;
  state.DamageSamples++;
  if (state.DamageSamples < 2 || now - state.DamageAlertAt < Settings.CooldownSeconds * 1000) return;
  state.DamageAlertAt = now;
  string evidence = "Projectile " + ammo + " | Zone " + zone + " | Calculated health damage " + observed.ToString();
  evidence += " | Ammo base " + nominal.ToString() + " | Envelope multiplier " + Settings.DamageEnvelopeMultiplier.ToString();
  Report("gameplay_damage", attacker, evidence + " | Repeated outliers; custom damage mods/zone multipliers may explain this.", target);
 }
};
modded class PlayerBase
{
 ref GRZGameplayAlertState m_GRZGameplayAlerts;
 override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
 {
  super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
  GRZGameplayAlerts.Damage(this, damageResult, damageType, source, dmgZone, ammo);
 }
};
modded class Weapon_Base
{
 protected int m_GRZShotStart = -1;
 protected int m_GRZLastShot = -1;
 protected int m_GRZShotCount;
 protected int m_GRZFastWindows;
 protected float m_GRZShotInterval;
 protected string m_GRZShotOwner;
 protected int m_GRZShotMode = -1;
 override void OnFire(int muzzle_index)
 {
  super.OnFire(muzzle_index);
  if (!g_Game || !g_Game.IsServer()) return;
  PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
  GRZGameplayAlertState state = GRZGameplayAlerts.State(player);
  if (!state || !GRZGameplayAlerts.Settings.FireRateEnabled) return;
  int now = g_Game.GetTime();
  float interval = GetReloadTime(muzzle_index) * 1000.0;
  int mode = GetCurrentMode(muzzle_index);
  string owner = player.GetIdentity().GetId();
  // Skip burst/multi-muzzle weapons: their timing contracts differ.
  if (GetMuzzleCount() != 1 || GetCurrentModeBurstSize(muzzle_index) > 1 || interval < 10 || now < state.GraceUntil)
  {
   m_GRZShotStart = -1; m_GRZFastWindows = 0;
   return;
  }
  if (m_GRZShotStart < 0 || owner != m_GRZShotOwner || mode != m_GRZShotMode || interval != m_GRZShotInterval || now - m_GRZLastShot > Math.Max(5000.0, interval * 3.0))
  {
   m_GRZShotOwner = owner; m_GRZShotMode = mode; m_GRZShotInterval = interval;
   m_GRZShotStart = now; m_GRZShotCount = 1; m_GRZFastWindows = 0; m_GRZLastShot = now;
   return;
  }
  m_GRZLastShot = now;
  m_GRZShotCount++;
  int elapsed = now - m_GRZShotStart;
  if (elapsed < 2000 || m_GRZShotCount < 6) return;
  if (GRZAlertRules.FastCadence(m_GRZShotCount, elapsed, interval, GRZGameplayAlerts.Settings.FireIntervalRatio)) m_GRZFastWindows++;
  else m_GRZFastWindows = 0;
  if (m_GRZFastWindows >= GRZGameplayAlerts.Settings.ConsecutiveFireWindows && now - state.FireAlertAt >= GRZGameplayAlerts.Settings.CooldownSeconds * 1000)
  {
   state.FireAlertAt = now;
   string evidence = "Weapon " + GetType() + " | Shots " + m_GRZShotCount.ToString() + " in " + elapsed.ToString() + " ms";
   evidence += " | Configured interval " + interval.ToString() + " ms | Fast windows " + m_GRZFastWindows.ToString();
   GRZGameplayAlerts.Report("gameplay_fire_rate", player, evidence + " | Server shot callbacks; replication/buffering may explain this.");
  }
  m_GRZShotStart = now; m_GRZShotCount = 1;
 }
};
