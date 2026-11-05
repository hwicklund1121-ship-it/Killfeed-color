class GRZClothingLabels
{
 static string Label(string type, string displayName)
 {
  // Use named variants; do not guess colours from image pixels or opaque skin IDs.
  string identity = type;
  array<string> textures = new array<string>;
  g_Game.ConfigGetTextArray("CfgVehicles " + type + " hiddenSelectionsTextures", textures);
  foreach (string texture : textures)
  {
   texture.Replace("\\", "/");
   array<string> parts = new array<string>;
   texture.Split("/", parts);
   if (parts.Count() > 0)
    identity += "_" + parts[parts.Count() - 1];
  }
  identity.ToLower();
  identity.Replace("gray", "grey");
  identity.Replace("_", "");
  identity.Replace("-", "");
  identity.Replace(" ", "");
  array<string> keys = {"darkgreen", "darkblue", "darkred", "lightblue", "lightgreen", "black", "white", "grey", "red", "blue", "green", "yellow", "orange", "pink", "purple", "brown", "beige", "olive", "khaki", "navy", "maroon", "violet", "turquoise", "cyan", "magenta", "cream", "gold", "silver", "tan"};
  array<string> labels = {"Dark Green", "Dark Blue", "Dark Red", "Light Blue", "Light Green", "Black", "White", "Grey", "Red", "Blue", "Green", "Yellow", "Orange", "Pink", "Purple", "Brown", "Beige", "Olive", "Khaki", "Navy", "Maroon", "Violet", "Turquoise", "Cyan", "Magenta", "Cream", "Gold", "Silver", "Tan"};
  array<string> found = new array<string>;
  for (int i = 0; i < keys.Count(); i++)
  {
   if (!identity.Contains(keys[i]))
    continue;
   found.Insert(labels[i]);
   identity.Replace(keys[i], "");
  }
  string suffix;
  foreach (string color : found)
  {
   if (suffix != "")
    suffix += " / ";
   suffix += color;
  }
  if (identity.Contains("woodland"))
  {
   if (suffix != "")
    suffix += " / ";
   suffix += "Woodland";
  }
  if (identity.Contains("check"))
  {
   if (suffix != "")
    suffix += " / ";
   suffix += "Checked";
  }
  if (suffix == "")
  {
   string lowerType = type;
   lowerType.ToLower();
   suffix = "Default";
   if (lowerType.IndexOf("grz_") == 0 || lowerType.IndexOf("blake_") == 0)
    suffix = "Custom";
  }
  return Widget.TranslateString(displayName) + " [" + suffix + "]";
 }
};
