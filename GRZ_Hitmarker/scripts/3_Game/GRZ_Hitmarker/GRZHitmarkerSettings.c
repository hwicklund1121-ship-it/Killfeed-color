class GRZHitmarkerSettings
{
    bool ShowHitmarkers = true;
    bool ShowDamage = true;
    bool PlaySound = true;
    bool HighlightKills;
    int BodyColor;
    int HeadColor = 1;
    int SizePercent = 50;
    int VolumePercent = 100;

    bool IsValid()
    {
        return BodyColor >= 0 && BodyColor < 12 && HeadColor >= 0 && HeadColor < 12 && SizePercent >= 25 && SizePercent <= 150 && SizePercent % 5 == 0 && VolumePercent >= 0 && VolumePercent <= 100 && VolumePercent % 5 == 0;
    }

    GRZHitmarkerSettings Clone()
    {
        GRZHitmarkerSettings result = new GRZHitmarkerSettings;
        result.ShowHitmarkers = ShowHitmarkers;
        result.ShowDamage = ShowDamage;
        result.PlaySound = PlaySound;
        result.HighlightKills = HighlightKills;
        result.BodyColor = BodyColor;
        result.HeadColor = HeadColor;
        result.SizePercent = SizePercent;
        result.VolumePercent = VolumePercent;
        return result;
    }

    bool SameAs(GRZHitmarkerSettings other)
    {
        return other && ShowHitmarkers == other.ShowHitmarkers && ShowDamage == other.ShowDamage && PlaySound == other.PlaySound && HighlightKills == other.HighlightKills && BodyColor == other.BodyColor && HeadColor == other.HeadColor && SizePercent == other.SizePercent && VolumePercent == other.VolumePercent;
    }

    static int Color(int index)
    {
        switch (index)
        {
            case 1: return 0xFFFF4545;
            case 2: return 0xFFFF9B39;
            case 3: return 0xFFFFE34A;
            case 4: return 0xFF3BDF70;
            case 5: return 0xFF4089FF;
            case 6: return 0xFF6344DB;
            case 7: return 0xFFC66EFF;
            case 8: return 0xFF67E7FF;
            case 9: return 0xFFFF80C8;
            case 10: return 0xFF9E9E9E;
            case 11: return 0xFF000000;
        }
        return 0xFFFFFFFF;
    }

    static string ColorName(int index)
    {
        array<string> names = {"WHITE", "RED", "ORANGE", "YELLOW", "GREEN", "BLUE", "INDIGO", "VIOLET", "CYAN", "PINK", "GREY", "BLACK"};
        if (index >= 0 && index < names.Count())
            return names[index];
        return "WHITE";
    }
}

class GRZHitmarkerStore
{
    protected static ref GRZHitmarkerSettings s_Current;
    static const string PATH = "$profile:GRZ_Hitmarker/settings.json";

    static GRZHitmarkerSettings Current()
    {
        if (!s_Current)
        {
            s_Current = new GRZHitmarkerSettings;
            if (!g_Game.IsDedicatedServer() && FileExist(PATH))
            {
                string error;
                GRZHitmarkerSettings loaded;
                if (JsonFileLoader<GRZHitmarkerSettings>.LoadFile(PATH, loaded, error) && loaded && loaded.IsValid())
                    s_Current = loaded;
                else
                    Error("GRZ Hitmarker: settings could not be loaded; using defaults.");
            }
        }
        return s_Current;
    }

    static bool Save(GRZHitmarkerSettings settings)
    {
        if (!settings || !settings.IsValid() || g_Game.IsDedicatedServer())
            return false;
        MakeDirectory("$profile:GRZ_Hitmarker");
        string error;
        if (!JsonFileLoader<GRZHitmarkerSettings>.SaveFile(PATH, settings, error))
        {
            Error("GRZ Hitmarker: could not save client settings: " + error);
            return false;
        }
        s_Current = settings.Clone();
        return true;
    }
}

class GRZHitmarkerEvent
{
    float DamageAmount;
    bool IsHeadHit;
    bool IsKill;
}

class GRZHitmarkerBus
{
    static const int HIT_RPC = 1647280201;
    static ref ScriptInvoker Hits = new ScriptInvoker;
}
