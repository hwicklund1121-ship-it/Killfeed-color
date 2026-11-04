class GRZBetterAimSettings
{
    static const float PROBE_DISTANCE = 250.0;
    static const float PROBE_RADIUS = 0.03;
    static const float ALIGNMENT_COSINE = 0.85;
}

modded class DayZPlayerTypeUtils
{
    override static void DayZPlayerTypeCreate(DayZPlayerType playerType)
    {
        if (!playerType)
            return;
        super.DayZPlayerTypeCreate(playerType);
        // A modest native alignment adjustment; damage and camera motion stay native.
        playerType.SetCameraShootParams(GRZBetterAimSettings.PROBE_DISTANCE, GRZBetterAimSettings.PROBE_RADIUS, GRZBetterAimSettings.ALIGNMENT_COSINE);
    }
}
