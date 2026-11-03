#ifndef SERVER
modded class MissionGameplay
{
    protected float m_GRZVisibilityCheck = 1;

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        m_GRZVisibilityCheck += timeslice;
        if (m_GRZVisibilityCheck < 1 || !g_Game.GetPlayer() || !g_Game.GetWorld())
            return;
        m_GRZVisibilityCheck = 0;
        // Session rendering only: do not persist preferred graphics settings.
        g_Game.GetWorld().SetViewDistance(1400);
        g_Game.GetWorld().SetObjectViewDistance(1400);
        Weather weather = g_Game.GetWeather();
        weather.GetFog().SetLimits(0, 0);
        weather.GetFog().Set(0, 0, 86400);
        weather.SetDynVolFogDistanceDensity(0, 0);
        weather.SetDynVolFogHeightDensity(0, 0);
    }
}
#endif
