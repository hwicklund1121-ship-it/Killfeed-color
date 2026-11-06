class GRZHexgridAnimationTicker
{
 protected static ref array<GRZ_Hexgrid_Animated_Anime> s_Items = new array<GRZ_Hexgrid_Animated_Anime>;
 protected static bool s_Running;
 protected static int s_Frame;
 static const int FRAME_COUNT = 119;
 static void Register(GRZ_Hexgrid_Animated_Anime item)
 {
  if (!g_Game || g_Game.IsDedicatedServer() || !item) return;
  if (s_Items.Find(item) < 0) s_Items.Insert(item);
  item.GRZRenderHexFrame(s_Frame);
  if (s_Running) return;
  s_Running = true;
  g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(Tick, 125, true);
 }
 static void Unregister(GRZ_Hexgrid_Animated_Anime item)
 {
  int index = s_Items.Find(item);
  if (index >= 0) s_Items.Remove(index);
  if (s_Items.Count() == 0) Stop();
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
  s_Frame = (s_Frame + 1) % FRAME_COUNT;
  for (int i = s_Items.Count() - 1; i >= 0; i--)
  {
   GRZ_Hexgrid_Animated_Anime item = s_Items[i];
   if (!item) s_Items.Remove(i);
   else item.GRZRenderHexFrame(s_Frame);
  }
  if (s_Items.Count() == 0) Stop();
 }
};
class GRZ_Hexgrid_Animated_Anime: Clothing
{
 protected int m_GRZHexFrame = -1;
 override void EEInit()
 {
  super.EEInit();
  GRZHexgridAnimationTicker.Register(this);
 }
 override void EEOnAfterLoad()
 {
  super.EEOnAfterLoad();
  GRZHexgridAnimationTicker.Register(this);
 }
 void GRZRenderHexFrame(int frame)
 {
  if (m_GRZHexFrame == frame) return;
  m_GRZHexFrame = frame;
  string number = frame.ToString();
  if (frame < 10) number = "00" + number;
  else if (frame < 100) number = "0" + number;
  SetObjectTexture(0, "GRZ_AnimatedHexgrid/data/frame_" + number + ".paa");
 }
 override void EEDelete(EntityAI parent)
 {
  GRZHexgridAnimationTicker.Unregister(this);
  super.EEDelete(parent);
 }
};
