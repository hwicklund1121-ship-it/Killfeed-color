#ifndef SERVER
modded class MissionGameplay
{
    // Time starts when the local player actually enters the optics camera.
    static const float GRZ_SCOPE_GRACE_SECONDS = 0.75;
    protected float m_GRZScopeAge;
    protected PlayerBase m_GRZScopePlayer;
    protected Weapon_Base m_GRZScopeWeapon;
    protected Widget m_GRZScopeBlackout;
    protected bool m_GRZScopeLayoutAttempted;

    protected void GRZResetScopeTimer()
    {
        m_GRZScopeAge = 0;
        m_GRZScopePlayer = null;
        m_GRZScopeWeapon = null;
        if (m_GRZScopeBlackout)
            m_GRZScopeBlackout.Show(false);
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
        // Holding a weapon excludes binoculars and handheld observation optics.
        Weapon_Base weapon;
        if (player)
            weapon = Weapon_Base.Cast(player.GetHumanInventory().GetEntityInHands());
        if (!player || !player.IsAlive() || player.IsUnconscious() || !weapon || !player.IsInOptics())
        {
            GRZResetScopeTimer();
            return;
        }
        if (m_GRZScopePlayer != player || m_GRZScopeWeapon != weapon)
        {
            GRZResetScopeTimer();
            m_GRZScopePlayer = player;
            m_GRZScopeWeapon = weapon;
        }
        m_GRZScopeAge += Math.Max(0, timeslice);
        bool blackedOut = m_GRZScopeAge >= GRZ_SCOPE_GRACE_SECONDS;
        // Menus stay usable, but opening one does not reset the scope timer.
        bool showBlackout = blackedOut && !g_Game.GetUIManager().GetMenu() && !IsPaused();
        if (showBlackout && !m_GRZScopeLayoutAttempted)
        {
            m_GRZScopeLayoutAttempted = true;
            // Independent of the HUD visibility toggle; never capture mouse/keyboard.
            m_GRZScopeBlackout = g_Game.GetWorkspace().CreateWidgets("GRZ_AntiHardscope/gui/layouts/blackout.layout");
            if (m_GRZScopeBlackout)
                m_GRZScopeBlackout.SetSort(1000);
            else
                Print("[GRZ Anti Hardscope] Blackout layout could not be loaded.");
        }
        if (m_GRZScopeBlackout)
            m_GRZScopeBlackout.Show(showBlackout);
    }

    override void OnMissionFinish()
    {
        GRZResetScopeTimer();
        if (m_GRZScopeBlackout)
        {
            m_GRZScopeBlackout.Unlink();
            m_GRZScopeBlackout = null;
        }
        super.OnMissionFinish();
    }
}
#endif
