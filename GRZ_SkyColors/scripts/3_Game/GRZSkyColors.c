class GRZSkyColor
{
    string Name;
    string Key;
    ref array<float> RGB;
}

class GRZSkyPaletteData
{
    ref array<ref GRZSkyColor> PaletteEntries;
}

class GRZSkyPalette
{
    protected static ref GRZSkyPaletteData s_Data;

    static GRZSkyPaletteData Current()
    {
        if (!s_Data)
        {
            string error;
            if (!JsonFileLoader<GRZSkyPaletteData>.LoadFile("GRZ_SkyColors/data/palette.json", s_Data, error) || !s_Data || !s_Data.PaletteEntries || s_Data.PaletteEntries.Count() != 11)
            {
                s_Data = new GRZSkyPaletteData;
                s_Data.PaletteEntries = new array<ref GRZSkyColor>;
                GRZSkyColor fallback = new GRZSkyColor;
                fallback.Name = "DEFAULT";
                fallback.Key = "default";
                s_Data.PaletteEntries.Insert(fallback);
                Error("GRZ Sky colors: palette failed to load: " + error);
            }
        }
        return s_Data;
    }

    static bool Valid(int index)
    {
        return index >= 0 && index < Current().PaletteEntries.Count();
    }
}

class GRZSkySettings
{
    int Color;

    bool IsValid()
    {
        return GRZSkyPalette.Valid(Color);
    }
}

class GRZSkyStore
{
    protected static ref GRZSkySettings s_Current;
    static const string PATH = "$profile:GRZ_SkyColors/settings.json";

    static GRZSkySettings Current()
    {
        if (!s_Current)
        {
            s_Current = new GRZSkySettings;
            if (!g_Game.IsDedicatedServer() && FileExist(PATH))
            {
                GRZSkySettings loaded;
                string error;
                if (JsonFileLoader<GRZSkySettings>.LoadFile(PATH, loaded, error) && loaded && loaded.IsValid())
                    s_Current = loaded;
                else
                    Error("GRZ Sky colors: invalid settings; using DEFAULT.");
            }
        }
        return s_Current;
    }

    static bool Save(int color)
    {
        if (!GRZSkyPalette.Valid(color) || g_Game.IsDedicatedServer())
            return false;
        GRZSkySettings settings = new GRZSkySettings;
        settings.Color = color;
        MakeDirectory("$profile:GRZ_SkyColors");
        string error;
        if (!JsonFileLoader<GRZSkySettings>.SaveFile(PATH, settings, error))
        {
            Error("GRZ Sky colors: could not save client settings: " + error);
            return false;
        }
        s_Current = settings;
        return true;
    }
}

class GRZSkyRuntime
{
    protected static int s_Preview = -1;
    protected static string s_BasePath = "dz/data/lighting/lighting_default.txt";
    protected static int s_BaseId;
    protected static string s_AppliedPath;

    static void SetBase(string path, int id)
    {
        s_BasePath = path;
        s_BaseId = id;
        s_AppliedPath = ""; // The native server-lighting handler just reloaded it.
    }

    static bool Supported()
    {
        return s_BaseId == 0 || s_BaseId == 1;
    }

    static bool Apply(int color)
    {
        if (!g_Game || g_Game.IsDedicatedServer() || !g_Game.GetWorld() || !GRZSkyPalette.Valid(color))
            return false;
        string path = s_BasePath;
        if (color != 0)
        {
            if (!Supported())
                return false;
            string base = "default";
            if (s_BaseId == 1)
                base = "darknight";
            path = "GRZ_SkyColors/data/lighting/" + base + "_" + GRZSkyPalette.Current().PaletteEntries[color].Key + ".txt";
        }
        path.Replace("/", "\\");
        if (!FileExist(path))
        {
            Error("GRZ Sky colors: lighting resource not found: " + path);
            return false;
        }
        if (s_AppliedPath != path)
        {
            // Native sky fields change; cloud illumination and scene light
            // values in these tables are identical to the supplied base file.
            g_Game.GetWorld().LoadNewLightingCfg(path);
            s_AppliedPath = path;
            Print("[GRZ Sky colors 0.1.2] Requested lighting: " + path);
        }
        return true;
    }

    static bool ApplyCurrent()
    {
        if (s_Preview >= 0)
            return Apply(s_Preview);
        return Apply(GRZSkyStore.Current().Color);
    }

    static bool Preview(int color)
    {
        if (!GRZSkyPalette.Valid(color))
            return false;
        s_Preview = color;
        return Apply(color);
    }

    static void EndPreview()
    {
        s_Preview = -1;
        ApplyCurrent();
    }

    static void RestoreBase()
    {
        s_Preview = -1;
        Apply(0);
    }
}

modded class WorldLighting
{
    override void SetGlobalLighting(int lightingID)
    {
        super.SetGlobalLighting(lightingID);
        if (g_Game.IsDedicatedServer())
            return;
        string path;
        switch (lightingID)
        {
            case 0: path = lighting_default; break;
            case 1: path = lighting_darknight; break;
            case 2: path = lighting_sakhal; break;
            case 3: path = lighting_nasdara; break;
            default: return;
        }
        GRZSkyRuntime.SetBase(path, lightingID);
        GRZSkyRuntime.ApplyCurrent();
    }
}
