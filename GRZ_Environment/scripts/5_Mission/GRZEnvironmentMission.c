modded class MissionServer
{
    protected float m_GRZEnvironmentCheck;

    protected void GRZLockEnvironment()
    {
        if (!g_Game.IsServer())
            return;
        g_Game.GetWorld().SetDate(2026, 7, 1, 12, 0);
        g_Game.GetWorld().SetTimeMultiplier(0);
        Weather weather = g_Game.GetWeather();
        weather.MissionWeather(false);
        weather.GetOvercast().SetLimits(0.25, 0.25);
        weather.GetOvercast().SetForecastChangeLimits(0, 0);
        weather.GetOvercast().Set(0.25, 0, 86400);
        weather.GetRain().SetLimits(0, 0);
        weather.GetRain().SetForecastChangeLimits(0, 0);
        weather.GetRain().Set(0, 0, 86400);
        weather.GetFog().SetLimits(0, 0);
        weather.GetFog().SetForecastChangeLimits(0, 0);
        weather.GetFog().Set(0, 0, 86400);
        weather.GetSnowfall().SetLimits(0, 0);
        weather.GetSnowfall().SetForecastChangeLimits(0, 0);
        weather.GetSnowfall().Set(0, 0, 86400);
        weather.GetWindMagnitude().SetLimits(0, 0);
        weather.GetWindMagnitude().SetForecastChangeLimits(0, 0);
        weather.GetWindMagnitude().Set(0, 0, 86400);
        weather.SetWindFunctionParams(0, 0, 0);
        weather.SetWindMaximumSpeed(0);
        weather.SetWindSpeed(0);
        weather.SetStorm(0, 1, 86400);
        weather.SetDynVolFogDistanceDensity(0, 0);
        weather.SetDynVolFogHeightDensity(0, 0);
        weather.SetWeatherUpdateFreeze(true);
    }

    override void OnMissionStart()
    {
        super.OnMissionStart();
        GRZLockEnvironment();
        Print("[GRZ Environment] Fixed noon, 25% clouds, no changing weather or dynamic PO-X strikes.");
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        m_GRZEnvironmentCheck += timeslice;
        if (m_GRZEnvironmentCheck >= 30)
        {
            m_GRZEnvironmentCheck = 0;
            // Reassert after mission weather initialization or admin changes.
            GRZLockEnvironment();
        }
    }
}
