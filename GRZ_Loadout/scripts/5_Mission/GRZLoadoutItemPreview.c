// One normalized preview widget and native setup shared by Loadout and GRZ VIP.
class GRZLoadoutItemPreview
{
 static ItemPreviewWidget Create(Widget parent)
 {
  if (!parent) return null;
  return ItemPreviewWidget.Cast(g_Game.GetWorkspace().CreateWidgets("GRZ_Loadout/gui/layouts/item_preview.layout", parent));
 }
 static EntityAI CreateItem(string type)
 {
  int flags = ECE_LOCAL | ECE_NOLIFETIME | ECE_KEEPHEIGHT | ECE_NOSURFACEALIGN | ECE_NOPERSISTENCY_WORLD | ECE_NOPERSISTENCY_CHAR;
  return EntityAI.Cast(g_Game.CreateObjectEx(type, "0 -1000 0", flags));
 }
 static void Bind(ItemPreviewWidget preview, EntityAI item)
 {
  if (!preview || !item) return;
  preview.SetItem(item);
  preview.SetView(item.GetViewIndex());
  preview.SetModelPosition("0 0 0.8");
  preview.SetModelOrientation("0 0 0");
 }
};
