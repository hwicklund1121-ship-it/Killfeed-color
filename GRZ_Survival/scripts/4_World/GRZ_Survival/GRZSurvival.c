class GRZSurvival
{
    static void Maintain(PlayerBase player)
    {
        if (!g_Game.IsServer() || !player || !player.IsAlive())
            return;
        player.GetStatEnergy().Set(player.GetStatEnergy().GetMax());
        player.GetStatWater().Set(player.GetStatWater().GetMax());
        player.GetStatHeatComfort().Set(0);
        player.GetStatHeatBuffer().Set(0);
        if (player.GetThermalBiasHandler())
            player.GetThermalBiasHandler().Set(0);
        player.UnsetMixedSoundState(eMixedSoundStates.HUNGRY);
        player.UnsetMixedSoundState(eMixedSoundStates.THIRSTY);
    }
}

modded class PlayerBase
{
    override void OnScheduledTick(float deltaTime)
    {
        // Clamp before modifiers so starvation/dehydration cannot deal damage,
        // and again after environment/disease processing to keep stats stable.
        GRZSurvival.Maintain(this);
        super.OnScheduledTick(deltaTime);
        GRZSurvival.Maintain(this);
    }
}

modded class HungerMdfr
{
    override void OnTick(PlayerBase player, float deltaT)
    {
        // Do not run native metabolic/starvation damage on the server.
        if (g_Game.IsServer())
            GRZSurvival.Maintain(player);
        else
            super.OnTick(player, deltaT);
    }
}

modded class ThirstMdfr
{
    override void OnTick(PlayerBase player, float deltaT)
    {
        if (g_Game.IsServer())
            GRZSurvival.Maintain(player);
        else
            super.OnTick(player, deltaT);
    }
}

modded class HeatComfortMdfr
{
    override void OnTick(PlayerBase player, float deltaT)
    {
        // Prevent both cold health loss and heat-stroke thermal-bias buildup.
        if (g_Game.IsServer())
            GRZSurvival.Maintain(player);
        else
            super.OnTick(player, deltaT);
    }
}

modded class Environment
{
    protected void GRZNeutralTemperature()
    {
        m_HeatComfort = 0;
        m_TargetHeatComfort = 0;
        m_TargetHeatComfortRaw = 0;
        GRZSurvival.Maintain(m_Player);
    }

    override protected void ProcessHeatComfort()
    {
        if (g_Game.IsServer())
            GRZNeutralTemperature();
        else
            super.ProcessHeatComfort();
    }

    override protected void SetHeatcomfortDirectly()
    {
        if (g_Game.IsServer())
            GRZNeutralTemperature();
        else
            super.SetHeatcomfortDirectly();
    }
}

modded class DayZPlayerImplementFallDamage
{
    override void HandleFallDamage(FallDamageData pData)
    {
        // Suppress the fall-specific handler only. This also prevents fall
        // shock, broken legs, bleeding and damage to attached gear.
        // Normal landing/movement processing continues in the caller.
    }
}
