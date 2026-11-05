// Explicit solid textures keep menu panels visible without engine-specific panel styles.
class GRZLoadoutPanelTextures
{
 static void Apply(Widget root)
 {
  if (!root) return;
  ImageWidget image = ImageWidget.Cast(root);
  if (image && root.GetName() != "Logo")
  {
   image.LoadImageFile(0, "GRZ_Loadout/data/solid_white.paa");
   image.SetImage(0);
   image.SetFlags(WidgetFlags.SOURCEALPHA | WidgetFlags.BLEND);
  }
  Widget child = root.GetChildren();
  while (child)
  {
   Apply(child);
   child = child.GetSibling();
  }
 }
};
