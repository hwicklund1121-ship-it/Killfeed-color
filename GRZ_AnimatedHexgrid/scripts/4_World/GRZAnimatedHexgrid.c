class GRZHexgridAnimationTicker
{
 protected static ref array<GRZ_Hexgrid_Animated_Anime> s_Items = new array<GRZ_Hexgrid_Animated_Anime>;
 protected static ref array<ImageWidget> s_CacheWidgets = new array<ImageWidget>;
 protected static ref array<bool> s_FrameLoaded = new array<bool>;
 protected static bool s_Running;
 protected static int s_Frame;
 static const int FRAME_COUNT = 119;
 static string FramePath(int frame)
 {
  string number = frame.ToString();
  if (frame < 10) number = "00" + number;
  else if (frame < 100) number = "0" + number;
  return "GRZ_AnimatedHexgrid/data/frame_" + number + ".paa";
 }
 static bool Ready(int frame)
 {
  if (frame < 0 || frame >= s_FrameLoaded.Count()) return false;
  return s_FrameLoaded[frame];
 }
 static void WarmCache()
 {
  if (s_FrameLoaded.Count() > 0) return;
  int loadedCount;
  ImageWidget cache;
  // ImageWidget supports slots 0..7. Keep a separate owner for each group of eight.
  for (int frame = 0; frame < FRAME_COUNT; frame++)
  {
   int slot = frame % 8;
   if (slot == 0)
   {
    cache = ImageWidget.Cast(g_Game.GetWorkspace().CreateWidgets("GRZ_AnimatedHexgrid/gui/layouts/frame_cache.layout"));
    if (cache)
    {
     cache.Show(false);
     s_CacheWidgets.Insert(cache);
    }
   }
   bool loaded = false;
   if (cache) loaded = cache.LoadImageFile(slot, FramePath(frame), false);
   s_FrameLoaded.Insert(loaded);
   if (loaded) loadedCount++;
  }
  Print(string.Format("[GRZ Animated Hexgrid] Cached %1/%2 animation frames.", loadedCount, FRAME_COUNT));
 }
 static void Register(GRZ_Hexgrid_Animated_Anime item)
 {
  if (!g_Game || g_Game.IsDedicatedServer() || !item) return;
  WarmCache();
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
  foreach (ImageWidget cached : s_CacheWidgets)
  {
   if (cached) cached.Unlink();
  }
  s_CacheWidgets.Clear();
  s_FrameLoaded.Clear();
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
  if (g_Game && !g_Game.IsDedicatedServer()) SetObjectMaterial(0, "GRZ_AnimatedHexgrid/data/flat_black.rvmat");
  GRZHexgridAnimationTicker.Register(this);
 }
 override void EEOnAfterLoad()
 {
  super.EEOnAfterLoad();
  if (g_Game && !g_Game.IsDedicatedServer()) SetObjectMaterial(0, "GRZ_AnimatedHexgrid/data/flat_black.rvmat");
  GRZHexgridAnimationTicker.Register(this);
 }
 void GRZRenderHexFrame(int frame)
 {
  if (m_GRZHexFrame == frame || !GRZHexgridAnimationTicker.Ready(frame)) return;
  m_GRZHexFrame = frame;
  SetObjectTexture(0, GRZHexgridAnimationTicker.FramePath(frame));
 }
 override void EEDelete(EntityAI parent)
 {
  GRZHexgridAnimationTicker.Unregister(this);
  super.EEDelete(parent);
 }
};
