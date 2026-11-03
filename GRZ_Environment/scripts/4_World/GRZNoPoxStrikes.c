// The native CE entry point sends the artillery RPC before creating its timers.
// Suppress it, including restored dynamic areas, before any gas/FX can start.
// Permanent static zones and player-thrown gas grenades use other classes.
modded class ContaminatedArea_Dynamic
{
    protected void GRZDiscardStrike()
    {
        if (g_Game.IsServer())
            g_Game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(g_Game.ObjectDelete, 1, false, this);
    }

    override void EEOnCECreate()
    {
        GRZDiscardStrike();
    }

    override void DeferredInit()
    {
        GRZDiscardStrike();
    }

    override void InitZone()
    {
        GRZDiscardStrike();
    }

    override void OnVariablesSynchronized()
    {
        // No airborne explosion/flare effects for a suppressed dynamic area.
    }
}
