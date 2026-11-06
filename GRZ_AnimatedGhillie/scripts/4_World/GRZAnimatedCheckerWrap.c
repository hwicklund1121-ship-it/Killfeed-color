// One client-cosmetic timer serves every wrap, including local menu previews.
// No server movement, inventory updates or recurring texture RPCs are needed.
class GRZCheckerWrapTicker
{
 protected static ref array<GRZ_Animated_Ghillie_Checker> s_Wraps = new array<GRZ_Animated_Ghillie_Checker>;
 protected static bool s_Running;
 protected static int s_Frame;
 static const int INTERVAL_MS = 250;

 static void Register(GRZ_Animated_Ghillie_Checker wrap)
 {
  if (!g_Game || g_Game.IsDedicatedServer() || !wrap) return;
  if (s_Wraps.Find(wrap) < 0) s_Wraps.Insert(wrap);
  wrap.GRZRenderCheckerFrame(s_Frame);
  if (s_Running) return;
  s_Running = true;
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, INTERVAL_MS, true);
 }
 static void Unregister(GRZ_Animated_Ghillie_Checker wrap)
 {
  int index = s_Wraps.Find(wrap);
  if (index >= 0) s_Wraps.Remove(index);
  if (s_Wraps.Count() == 0) Stop();
 }
 static void Stop()
 {
  if (g_Game) g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(Tick);
  s_Running = false;
  s_Frame = 0;
 }
 static void Tick()
 {
  if (!g_Game) return;
  s_Frame = 1 - s_Frame;
  for (int i = s_Wraps.Count() - 1; i >= 0; i--)
  {
   GRZ_Animated_Ghillie_Checker wrap = s_Wraps[i];
   if (!wrap) s_Wraps.Remove(i);
   else wrap.GRZRenderCheckerFrame(s_Frame);
  }
  if (s_Wraps.Count() == 0) Stop();
 }
};
class GRZ_Animated_Ghillie_Checker: GhillieAtt_ColorBase
{
 protected int m_GRZCheckerFrame = -1;
 override void EEInit()
 {
  super.EEInit();
  GRZCheckerWrapTicker.Register(this);
 }
 override void EEOnAfterLoad()
 {
  super.EEOnAfterLoad();
  GRZCheckerWrapTicker.Register(this);
 }
 void GRZRenderCheckerFrame(int frame)
 {
  if (m_GRZCheckerFrame == frame) return;
  m_GRZCheckerFrame = frame;
  string texture = "GRZ_AnimatedGhillie/data/checker_a_co.paa";
  if (frame == 1) texture = "GRZ_AnimatedGhillie/data/checker_b_co.paa";
  SetObjectTexture(0, texture);
 }
 override void EEDelete(EntityAI parent)
 {
  GRZCheckerWrapTicker.Unregister(this);
  super.EEDelete(parent);
 }
};
