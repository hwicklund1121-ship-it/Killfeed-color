// Client camera/UI only. Dedicated servers use their native implementations.
#ifndef SERVER
// Client-only preferences. No server data or third-party UI resources.
class GRZCameraPreferences
{
    static const string FILE_PATH = "$profile:GRZMovementCamera.json";
    private static ref GRZCameraPreferences s_Current;
    private static bool s_HasFovPreview;
    private static float s_FovPreview;

    // Session-only slider preview. Never change saved preferences while dragging.
    static void PreviewFov(float value)
    {
        s_FovPreview = Math.Clamp(value, 0.52, 1.5599);
        s_HasFovPreview = true;
    }

    static void ClearFovPreview()
    {
        s_HasFovPreview = false;
    }

    static bool HasFovPreview()
    {
        return s_HasFovPreview;
    }

    static float FovPreview()
    {
        return s_FovPreview;
    }

    int Version = 1;
    bool Enabled = false;
    float FieldOfView = 1.0;
    float ViewDistance = 2000;
    float ObjectDistance = 2000;

    static GRZCameraPreferences Current()
    {
        if (!s_Current)
            s_Current = new GRZCameraPreferences();
        return s_Current;
    }

    void Validate()
    {
        FieldOfView = Math.Clamp(FieldOfView, 0.52, 1.5599);
        ViewDistance = Math.Clamp(ViewDistance, 500, 2000);
        ObjectDistance = Math.Clamp(ObjectDistance, 500, ViewDistance);
    }

    static void Load()
    {
        if (GetGame().IsDedicatedServer())
            return;
        ClearFovPreview();
        s_Current = new GRZCameraPreferences();
        if (!FileExist(FILE_PATH))
            return;
        GRZCameraPreferences loaded = new GRZCameraPreferences();
        string error;
        if (!JsonFileLoader<GRZCameraPreferences>.LoadFile(FILE_PATH, loaded, error) || !loaded || loaded.Version != 1)
        {
            Print("[GRZ SETTINGS] Could not load preferences; using game settings. " + error);
            return;
        }
        loaded.Validate();
        s_Current = loaded;
    }

    static bool SaveAndApply(GRZCameraPreferences preferences, out string error)
    {
        preferences.Validate();
        if (!JsonFileLoader<GRZCameraPreferences>.SaveFile(FILE_PATH, preferences, error))
            return false;
        s_Current = preferences;
        ClearFovPreview();
        ApplyDistances();
        return true;
    }

    static void ApplyDistances()
    {
        if (GetGame().IsDedicatedServer() || !Current().Enabled)
            return;
        World world = GetGame().GetWorld();
        if (world)
        {
            world.SetViewDistance(Current().ViewDistance);
            world.SetObjectViewDistance(Current().ObjectDistance);
        }
    }
}

modded class DayZGame
{
    override float GetUserFOV()
    {
        if (!IsDedicatedServer() && GRZCameraPreferences.HasFovPreview())
            return GRZCameraPreferences.FovPreview();
        if (!IsDedicatedServer() && GRZCameraPreferences.Current().Enabled)
            return GRZCameraPreferences.Current().FieldOfView;
        return super.GetUserFOV();
    }
}

#endif
