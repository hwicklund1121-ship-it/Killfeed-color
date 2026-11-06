// One client-cosmetic timer serves every wrap, including local menu previews.
// No server movement, inventory updates or recurring texture RPCs are needed.
class GRZCrossWrapTicker
{
 protected static ref array<GRZ_Animated_Ghillie_Cross> s_Wraps = new array<GRZ_Animated_Ghillie_Cross>;
 protected static bool s_Running;
 protected static int s_Frame;
 static const int INTERVAL_MS = 750;

 static void Register(GRZ_Animated_Ghillie_Cross wrap)
 {
  if (!g_Game || g_Game.IsDedicatedServer() || !wrap) return;
  if (s_Wraps.Find(wrap) < 0) s_Wraps.Insert(wrap);
  wrap.GRZRenderCrossFrame(s_Frame);
  if (s_Running) return;
  s_Running = true;
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, INTERVAL_MS, true);
 }
 static void Unregister(GRZ_Animated_Ghillie_Cross wrap)
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
   GRZ_Animated_Ghillie_Cross wrap = s_Wraps[i];
   if (!wrap) s_Wraps.Remove(i);
   else wrap.GRZRenderCrossFrame(s_Frame);
  }
  if (s_Wraps.Count() == 0) Stop();
 }
};
class GRZ_Animated_Ghillie_Cross: GhillieAtt_ColorBase
{
 protected int m_GRZCrossFrame = -1;
 override void EEInit()
 {
  super.EEInit();
  GRZCrossWrapTicker.Register(this);
 }
 override void EEOnAfterLoad()
 {
  super.EEOnAfterLoad();
  GRZCrossWrapTicker.Register(this);
 }
 void GRZRenderCrossFrame(int frame)
 {
  if (m_GRZCrossFrame == frame) return;
  m_GRZCrossFrame = frame;
  string texture = "GRZ_AnimatedGhillie/data/cross_upright_co.paa";
  if (frame == 1) texture = "GRZ_AnimatedGhillie/data/cross_inverted_co.paa";
  SetObjectTexture(0, texture);
 }
 override void EEDelete(EntityAI parent)
 {
  GRZCrossWrapTicker.Unregister(this);
  super.EEDelete(parent);
 }
};
