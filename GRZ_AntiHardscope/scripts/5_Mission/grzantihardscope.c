#ifndef SERVER
modded class MissionGameplay
{
    // Time starts when the local player actually enters the optics camera.
    static const float GRZ_SCOPE_GRACE_SECONDS = 0.75;
    protected float m_GRZScopeAge;
    protected PlayerBase m_GRZScopePlayer;
    protected Weapon_Base m_GRZScopeWeapon;
    protected Widget m_GRZScopeBlackout;
    protected ImageWidget m_GRZScopeLogo;
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
            {
                m_GRZScopeBlackout.SetSort(1000);
                m_GRZScopeLogo = ImageWidget.Cast(m_GRZScopeBlackout.FindAnyWidget("GRZScopeLogo"));
                if (m_GRZScopeLogo)
                {
                    m_GRZScopeLogo.LoadImage(0, "GRZ_AntiHardscope/gui/textures/grz_scope_logo_ca.paa");
                    m_GRZScopeLogo.SetImage(0);
                }
            }
            else
                Print("[GRZ Anti Hardscope] Blackout layout could not be loaded.");
        }
        if (m_GRZScopeBlackout)
            m_GRZScopeBlackout.Show(showBlackout);
        if (showBlackout && m_GRZScopeLogo)
        {
            int width, height;
            g_Game.GetWorkspace().GetScreenSize(width, height);
            float scale = Math.Clamp(Math.Min(width / 1920.0, height / 1080.0), 0.5, 2.0);
            float logoWidth = 320 * scale;
            float logoHeight = 160 * scale;
            m_GRZScopeLogo.SetSize(logoWidth, logoHeight);
            m_GRZScopeLogo.SetPos((width - logoWidth) * 0.5, (height - logoHeight) * 0.5);
        }
    }

    override void OnMissionFinish()
    {
        GRZResetScopeTimer();
        m_GRZScopeLogo = null;
        if (m_GRZScopeBlackout)
        {
            m_GRZScopeBlackout.Unlink();
            m_GRZScopeBlackout = null;
        }
        super.OnMissionFinish();
    }
}
#endif
